#pragma once

#ifdef FE_WITH_VULKAN

#include <vulkan/vulkan.h>
#include <functional>

namespace FunEngine
{
	struct VulkanBufferResource
	{
		VkBuffer Buffer = VK_NULL_HANDLE;
		VkDeviceMemory Memory = VK_NULL_HANDLE;
		VkDeviceSize Size = 0;
		void* Mapped = nullptr;
	};

	struct VulkanImageResource
	{
		VkImage Image = VK_NULL_HANDLE;
		VkDeviceMemory Memory = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
	};

	namespace VulkanBuffer
	{
		uint32_t FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);

		VulkanBufferResource CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool persistentMap = false);
		void DestroyBuffer(VulkanBufferResource& buffer);

		// Host-visible, persistently mapped, coherent.
		VulkanBufferResource CreateHostBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

		// Device-local buffer filled through a staging copy.
		VulkanBufferResource CreateDeviceLocalBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage);

		void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

		// One-shot command buffer on the graphics queue; blocks until the GPU finishes.
		void ImmediateSubmit(const std::function<void(VkCommandBuffer)>& record);
		void Shutdown();

		VulkanImageResource CreateImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
			VkImageAspectFlags aspect, VkImageLayout initialTransitionTo = VK_IMAGE_LAYOUT_UNDEFINED);
		void DestroyImage(VulkanImageResource& image);

		void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect,
			VkImageLayout oldLayout, VkImageLayout newLayout);

		// Releases through the frame deletion queue when a context exists, immediately otherwise.
		void DeferredDestroyBuffer(VulkanBufferResource buffer);
		void DeferredDestroyImage(VulkanImageResource image);
	}
}

#endif
