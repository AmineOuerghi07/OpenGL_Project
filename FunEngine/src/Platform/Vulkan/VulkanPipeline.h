#pragma once

#ifdef FE_WITH_VULKAN

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include "FunEngine/Renderer/Buffer.h"

namespace FunEngine
{
	class VulkanShader;

	struct VulkanTargetInfo
	{
		VkRenderPass RenderPass = VK_NULL_HANDLE;
		uint32_t ColorAttachmentCount = 1;
		bool HasDepth = false;
		VkExtent2D Extent{};
		bool FlipViewportY = false;
	};

	struct VulkanPipelineKey
	{
		uint64_t ShaderID = 0;
		uint64_t LayoutHash = 0;
		VkRenderPass RenderPass = VK_NULL_HANDLE;
		uint32_t ColorAttachmentCount = 1;
		bool HasDepth = false;

		bool operator==(const VulkanPipelineKey& o) const
		{
			return ShaderID == o.ShaderID && LayoutHash == o.LayoutHash && RenderPass == o.RenderPass &&
				ColorAttachmentCount == o.ColorAttachmentCount && HasDepth == o.HasDepth;
		}
	};

	struct VulkanPipelineKeyHash
	{
		size_t operator()(const VulkanPipelineKey& k) const;
	};

	class VulkanPipelineCache
	{
	public:
		static VulkanPipelineCache& Get();

		VkPipeline GetOrCreate(const VulkanShader& shader, const std::vector<Ref<VertexBuffer>>& vertexBuffers,
			const VulkanTargetInfo& target);

		void InvalidateShader(uint64_t shaderID);
		void InvalidateRenderPass(VkRenderPass renderPass);
		void Shutdown();

	private:
		void EraseWhere(const std::function<bool(const VulkanPipelineKey&)>& predicate);

		std::unordered_map<VulkanPipelineKey, VkPipeline, VulkanPipelineKeyHash> m_Pipelines;
	};
}

#endif
