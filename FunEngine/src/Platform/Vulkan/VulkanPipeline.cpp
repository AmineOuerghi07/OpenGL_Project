#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanTextureRegistry.h"
#include "VulkanUtils.h"

namespace FunEngine
{
	namespace
	{
		static VkFormat ToVkFormat(ShaderDataType type)
		{
			switch (type)
			{
			case ShaderDataType::Float:  return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Mat3:   return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Mat4:   return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ShaderDataType::Int:    return VK_FORMAT_R32_SINT;
			case ShaderDataType::Int2:   return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:   return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:   return VK_FORMAT_R32G32B32A32_SINT;
			case ShaderDataType::Bool:   return VK_FORMAT_R8_UINT;
			default: break;
			}
			FE_CORE_ASSERT(false, "Unknown ShaderDataType");
			return VK_FORMAT_UNDEFINED;
		}

		static uint64_t HashCombine(uint64_t seed, uint64_t value)
		{
			return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
		}

		static uint64_t HashLayouts(const std::vector<Ref<VertexBuffer>>& vertexBuffers)
		{
			uint64_t hash = 1469598103934665603ULL;
			for (const auto& vb : vertexBuffers)
			{
				BufferLayout layout = vb->GetLayout();
				hash = HashCombine(hash, layout.GetStride());
				for (const auto& element : layout)
				{
					hash = HashCombine(hash, (uint64_t)element.Type);
					hash = HashCombine(hash, element.Offset);
				}
				hash = HashCombine(hash, 0xfeed);
			}
			return hash;
		}
	}

	size_t VulkanPipelineKeyHash::operator()(const VulkanPipelineKey& k) const
	{
		uint64_t h = HashCombine(k.ShaderID, k.LayoutHash);
		h = HashCombine(h, (uint64_t)(uintptr_t)k.RenderPass);
		h = HashCombine(h, k.ColorAttachmentCount);
		h = HashCombine(h, k.HasDepth ? 1 : 0);
		return (size_t)h;
	}

	VulkanPipelineCache& VulkanPipelineCache::Get()
	{
		static VulkanPipelineCache instance;
		return instance;
	}

	VkPipeline VulkanPipelineCache::GetOrCreate(const VulkanShader& shader, const std::vector<Ref<VertexBuffer>>& vertexBuffers,
		const VulkanTargetInfo& target)
	{
		VulkanPipelineKey key;
		key.ShaderID = shader.GetID();
		key.LayoutHash = HashLayouts(vertexBuffers);
		key.RenderPass = target.RenderPass;
		key.ColorAttachmentCount = target.ColorAttachmentCount;
		key.HasDepth = target.HasDepth;

		auto it = m_Pipelines.find(key);
		if (it != m_Pipelines.end())
			return it->second;

		std::vector<VkVertexInputBindingDescription> bindingDescs;
		std::vector<VkVertexInputAttributeDescription> attributeDescs;
		uint32_t location = 0;
		for (uint32_t binding = 0; binding < vertexBuffers.size(); ++binding)
		{
			BufferLayout layout = vertexBuffers[binding]->GetLayout();

			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = binding;
			bindingDesc.stride = layout.GetStride();
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			bindingDescs.push_back(bindingDesc);

			for (const auto& element : layout)
			{
				uint32_t rows = 1;
				uint32_t rowSize = element.Size;
				if (element.Type == ShaderDataType::Mat3) { rows = 3; rowSize = 12; }
				if (element.Type == ShaderDataType::Mat4) { rows = 4; rowSize = 16; }
				for (uint32_t row = 0; row < rows; ++row)
				{
					VkVertexInputAttributeDescription attr{};
					attr.location = location++;
					attr.binding = binding;
					attr.format = ToVkFormat(element.Type);
					attr.offset = element.Offset + row * rowSize;
					attributeDescs.push_back(attr);
				}
			}
		}

		VkPipelineShaderStageCreateInfo stages[2]{};
		stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[0].module = shader.GetVertexModule();
		stages[0].pName = "main";
		stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[1].module = shader.GetFragmentModule();
		stages[1].pName = "main";

		VkPipelineVertexInputStateCreateInfo vertexInput{};
		vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexBindingDescriptionCount = (uint32_t)bindingDescs.size();
		vertexInput.pVertexBindingDescriptions = bindingDescs.data();
		vertexInput.vertexAttributeDescriptionCount = (uint32_t)attributeDescs.size();
		vertexInput.pVertexAttributeDescriptions = attributeDescs.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo raster{};
		raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster.polygonMode = VK_POLYGON_MODE_FILL;
		raster.cullMode = VK_CULL_MODE_NONE;
		raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisample{};
		multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = target.HasDepth ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = target.HasDepth ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

		// Mirrors OpenGLRendererAPI: alpha blending on color 0; integer attachments (entity IDs) never blend.
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(target.ColorAttachmentCount);
		for (uint32_t i = 0; i < target.ColorAttachmentCount; ++i)
		{
			auto& a = blendAttachments[i];
			a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			if (i == 0 && (target.ColorAttachmentCount == 1 || VulkanDevice::Get().SupportsIndependentBlend()))
			{
				a.blendEnable = VK_TRUE;
				a.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				a.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				a.colorBlendOp = VK_BLEND_OP_ADD;
				a.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				a.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				a.alphaBlendOp = VK_BLEND_OP_ADD;
			}
		}

		VkPipelineColorBlendStateCreateInfo blend{};
		blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend.attachmentCount = (uint32_t)blendAttachments.size();
		blend.pAttachments = blendAttachments.data();

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic{};
		dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic.dynamicStateCount = 2;
		dynamic.pDynamicStates = dynamicStates;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = stages;
		pipelineInfo.pVertexInputState = &vertexInput;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &raster;
		pipelineInfo.pMultisampleState = &multisample;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &blend;
		pipelineInfo.pDynamicState = &dynamic;
		pipelineInfo.layout = VulkanTextureRegistry::Get().GetPipelineLayout();
		pipelineInfo.renderPass = target.RenderPass;
		pipelineInfo.subpass = 0;

		VkPipeline pipeline = VK_NULL_HANDLE;
		VulkanUtils::CheckVk(vkCreateGraphicsPipelines(VulkanDevice::Get().GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline),
			"vkCreateGraphicsPipelines");

		FE_CORE_TRACE("Vulkan pipeline created (shader {0}, {1} color attachments)", key.ShaderID, target.ColorAttachmentCount);
		m_Pipelines[key] = pipeline;
		return pipeline;
	}

	void VulkanPipelineCache::InvalidateShader(uint64_t shaderID)
	{
		EraseWhere([shaderID](const VulkanPipelineKey& key) { return key.ShaderID == shaderID; });
	}

	void VulkanPipelineCache::InvalidateRenderPass(VkRenderPass renderPass)
	{
		EraseWhere([renderPass](const VulkanPipelineKey& key) { return key.RenderPass == renderPass; });
	}

	void VulkanPipelineCache::EraseWhere(const std::function<bool(const VulkanPipelineKey&)>& predicate)
	{
		for (auto it = m_Pipelines.begin(); it != m_Pipelines.end();)
		{
			if (predicate(it->first))
			{
				VkPipeline pipeline = it->second;
				auto destroy = [pipeline]()
				{
					VkDevice d = VulkanDevice::Get().GetDevice();
					if (d != VK_NULL_HANDLE)
						vkDestroyPipeline(d, pipeline, nullptr);
				};
				if (auto* ctx = VulkanContext::Get())
					ctx->DeferDestroy(destroy);
				else
					destroy();
				it = m_Pipelines.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void VulkanPipelineCache::Shutdown()
	{
		VkDevice device = VulkanDevice::Get().GetDevice();
		if (device != VK_NULL_HANDLE)
		{
			for (auto& [key, pipeline] : m_Pipelines)
				vkDestroyPipeline(device, pipeline, nullptr);
		}
		m_Pipelines.clear();
	}
}

#endif
