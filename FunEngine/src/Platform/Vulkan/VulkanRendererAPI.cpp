#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanRendererAPI.h"
#include "VulkanContext.h"
#include "VulkanBuffers.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanTextureRegistry.h"
#include "FunEngine/Core/Application.h"
#include "FunEngine/Core/Window.h"

namespace FunEngine
{
	void VulkanRendererAPI::Init()
	{
		FE_CORE_INFO("VulkanRendererAPI::Init");
	}

	void VulkanRendererAPI::SetViewPort(uint32_t, uint32_t, uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return;

		if (auto* ctx = VulkanContext::Get())
			ctx->SetFramebufferResized(true);
	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
		if (auto* ctx = VulkanContext::Get())
			ctx->SetClearColor(color);
	}

	void VulkanRendererAPI::Clear()
	{
		if (auto* ctx = VulkanContext::Get())
			ctx->ClearCurrentTarget();
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		if (indexCount == 0)
			return;

		auto* ctx = VulkanContext::Get();
		if (!ctx || !ctx->IsFrameActive())
			return;

		const VulkanShader* shader = VulkanShader::GetCurrent();
		if (!shader || !shader->IsValid())
		{
			static bool warned = false;
			if (!warned)
			{
				FE_CORE_WARN("VulkanRendererAPI::DrawIndexed: no valid shader bound");
				warned = true;
			}
			return;
		}

		const auto& vertexBuffers = vertexArray->GetVertexBuffer();
		auto* indexBuffer = static_cast<VulkanIndexBuffer*>(vertexArray->GetIndexBuffer().get());
		if (vertexBuffers.empty() || !indexBuffer)
			return;

		ctx->EnsureRenderPass();
		VkCommandBuffer cmd = ctx->GetCurrentCommandBuffer();
		const VulkanTargetInfo target = ctx->GetCurrentTargetInfo();

		VkPipeline pipeline = VulkanPipelineCache::Get().GetOrCreate(*shader, vertexBuffers, target);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		auto& registry = VulkanTextureRegistry::Get();
		VkDescriptorSet descriptorSet = registry.AcquireDescriptorSet();
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, registry.GetPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

		VkViewport viewport{};
		viewport.width = (float)target.Extent.width;
		viewport.height = (float)target.Extent.height;
		viewport.maxDepth = 1.0f;
		if (target.FlipViewportY)
		{
			// Keeps GL-style (Y-up) projections working on the swapchain.
			viewport.y = viewport.height;
			viewport.height = -viewport.height;
		}
		VkRect2D scissor{ { 0, 0 }, target.Extent };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		std::vector<VkBuffer> buffers;
		std::vector<VkDeviceSize> offsets;
		buffers.reserve(vertexBuffers.size());
		offsets.reserve(vertexBuffers.size());
		for (const auto& vb : vertexBuffers)
		{
			auto* vkVB = static_cast<VulkanVertexBuffer*>(vb.get());
			buffers.push_back(vkVB->GetBuffer());
			offsets.push_back(vkVB->GetBindOffset());
		}
		vkCmdBindVertexBuffers(cmd, 0, (uint32_t)buffers.size(), buffers.data(), offsets.data());
		vkCmdBindIndexBuffer(cmd, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
	}
}

#endif
