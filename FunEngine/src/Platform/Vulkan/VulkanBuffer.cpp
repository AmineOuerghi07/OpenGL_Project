#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

#include <cstring>

namespace FunEngine
{
	namespace VulkanBuffer
	{
		static VkCommandPool s_ImmediatePool = VK_NULL_HANDLE;

		uint32_t FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties)
		{
			return VulkanDevice::Get().FindMemoryType(typeBits, properties);
		}

		VulkanBufferResource CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool persistentMap)
		{
			VkDevice device = VulkanDevice::Get().GetDevice();
			VulkanBufferResource result;
			result.Size = size;

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VulkanUtils::CheckVk(vkCreateBuffer(device, &bufferInfo, nullptr, &result.Buffer), "vkCreateBuffer");

			VkMemoryRequirements memReq;
			vkGetBufferMemoryRequirements(device, result.Buffer, &memReq);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memReq.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, properties);
			VulkanUtils::CheckVk(vkAllocateMemory(device, &allocInfo, nullptr, &result.Memory), "vkAllocateMemory(buffer)");
			vkBindBufferMemory(device, result.Buffer, result.Memory, 0);

			if (persistentMap)
				VulkanUtils::CheckVk(vkMapMemory(device, result.Memory, 0, VK_WHOLE_SIZE, 0, &result.Mapped), "vkMapMemory");

			return result;
		}

		void DestroyBuffer(VulkanBufferResource& buffer)
		{
			VkDevice device = VulkanDevice::Get().GetDevice();
			if (device == VK_NULL_HANDLE)
			{
				buffer = {};
				return;
			}
			if (buffer.Mapped)
				vkUnmapMemory(device, buffer.Memory);
			if (buffer.Buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(device, buffer.Buffer, nullptr);
			if (buffer.Memory != VK_NULL_HANDLE)
				vkFreeMemory(device, buffer.Memory, nullptr);
			buffer = {};
		}

		VulkanBufferResource CreateHostBuffer(VkDeviceSize size, VkBufferUsageFlags usage)
		{
			return CreateBuffer(size, usage,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
		}

		VulkanBufferResource CreateDeviceLocalBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage)
		{
			VulkanBufferResource staging = CreateHostBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			if (data)
				memcpy(staging.Mapped, data, (size_t)size);
			else
				memset(staging.Mapped, 0, (size_t)size);

			VulkanBufferResource result = CreateBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			CopyBuffer(staging.Buffer, result.Buffer, size);
			DestroyBuffer(staging);
			return result;
		}

		void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
		{
			ImmediateSubmit([&](VkCommandBuffer cmd)
			{
				VkBufferCopy region{};
				region.size = size;
				vkCmdCopyBuffer(cmd, src, dst, 1, &region);
			});
		}

		void ImmediateSubmit(const std::function<void(VkCommandBuffer)>& record)
		{
			auto& vulkanDevice = VulkanDevice::Get();
			VkDevice device = vulkanDevice.GetDevice();

			if (s_ImmediatePool == VK_NULL_HANDLE)
			{
				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				poolInfo.queueFamilyIndex = vulkanDevice.GetGraphicsQueueFamily();
				VulkanUtils::CheckVk(vkCreateCommandPool(device, &poolInfo, nullptr, &s_ImmediatePool), "vkCreateCommandPool(immediate)");
			}

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = s_ImmediatePool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;
			VkCommandBuffer cmd = VK_NULL_HANDLE;
			VulkanUtils::CheckVk(vkAllocateCommandBuffers(device, &allocInfo, &cmd), "vkAllocateCommandBuffers(immediate)");

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(cmd, &beginInfo);
			record(cmd);
			vkEndCommandBuffer(cmd);

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			VkFence fence = VK_NULL_HANDLE;
			vkCreateFence(device, &fenceInfo, nullptr, &fence);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd;
			VulkanUtils::CheckVk(vkQueueSubmit(vulkanDevice.GetGraphicsQueue(), 1, &submitInfo, fence), "vkQueueSubmit(immediate)");
			vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

			vkDestroyFence(device, fence, nullptr);
			vkFreeCommandBuffers(device, s_ImmediatePool, 1, &cmd);
		}

		void Shutdown()
		{
			VkDevice device = VulkanDevice::Get().GetDevice();
			if (s_ImmediatePool != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
				vkDestroyCommandPool(device, s_ImmediatePool, nullptr);
			s_ImmediatePool = VK_NULL_HANDLE;
		}

		VulkanImageResource CreateImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
			VkImageAspectFlags aspect, VkImageLayout initialTransitionTo)
		{
			VkDevice device = VulkanDevice::Get().GetDevice();
			VulkanImageResource result;

			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent = { width, height, 1 };
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VulkanUtils::CheckVk(vkCreateImage(device, &imageInfo, nullptr, &result.Image), "vkCreateImage");

			VkMemoryRequirements memReq;
			vkGetImageMemoryRequirements(device, result.Image, &memReq);
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memReq.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VulkanUtils::CheckVk(vkAllocateMemory(device, &allocInfo, nullptr, &result.Memory), "vkAllocateMemory(image)");
			vkBindImageMemory(device, result.Image, result.Memory, 0);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = result.Image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = aspect;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.layerCount = 1;
			VulkanUtils::CheckVk(vkCreateImageView(device, &viewInfo, nullptr, &result.View), "vkCreateImageView");

			if (initialTransitionTo != VK_IMAGE_LAYOUT_UNDEFINED)
			{
				ImmediateSubmit([&](VkCommandBuffer cmd)
				{
					TransitionImageLayout(cmd, result.Image, aspect, VK_IMAGE_LAYOUT_UNDEFINED, initialTransitionTo);
				});
			}
			return result;
		}

		void DestroyImage(VulkanImageResource& image)
		{
			VkDevice device = VulkanDevice::Get().GetDevice();
			if (device == VK_NULL_HANDLE)
			{
				image = {};
				return;
			}
			if (image.View != VK_NULL_HANDLE)
				vkDestroyImageView(device, image.View, nullptr);
			if (image.Image != VK_NULL_HANDLE)
				vkDestroyImage(device, image.Image, nullptr);
			if (image.Memory != VK_NULL_HANDLE)
				vkFreeMemory(device, image.Memory, nullptr);
			image = {};
		}

		void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect,
			VkImageLayout oldLayout, VkImageLayout newLayout)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = aspect;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.layerCount = 1;

			// Conservative all-commands barrier: only used on cold paths (uploads, readback, creation).
			barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
				barrier.srcAccessMask = 0;

			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		void DeferredDestroyBuffer(VulkanBufferResource buffer)
		{
			if (auto* ctx = VulkanContext::Get())
				ctx->DeferDestroy([buffer]() mutable { DestroyBuffer(buffer); });
			else
				DestroyBuffer(buffer);
		}

		void DeferredDestroyImage(VulkanImageResource image)
		{
			if (auto* ctx = VulkanContext::Get())
				ctx->DeferDestroy([image]() mutable { DestroyImage(image); });
			else
				DestroyImage(image);
		}
	}
}

#endif
