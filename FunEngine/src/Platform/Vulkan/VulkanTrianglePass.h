#pragma once

#ifdef FE_WITH_VULKAN

#include <vulkan/vulkan.h>

namespace FunEngine
{
	// Hardcoded colored triangle drawn into the swapchain render pass (bring-up / debug aid).
	class VulkanTrianglePass
	{
	public:
		void Init(VkRenderPass renderPass);
		void Shutdown();
		void Record(VkCommandBuffer cmd, VkExtent2D extent) const;

	private:
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
	};
}

#endif
