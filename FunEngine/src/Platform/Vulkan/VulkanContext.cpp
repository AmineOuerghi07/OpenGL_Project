#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <algorithm>

namespace FunEngine
{
	namespace
	{
		static void CheckVk(VkResult result, const char* where)
		{
			if (result != VK_SUCCESS)
			{
				FE_CORE_ERROR("Vulkan error {0} at {1}", (int)result, where);
				FE_CORE_ASSERT(false, "Vulkan call failed");
			}
		}
	}

	VulkanContext* VulkanContext::s_Instance = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		FE_CORE_ASSERT(m_WindowHandle, "Window handle is null");
		s_Instance = this;
	}

	VulkanContext::~VulkanContext()
	{
		auto& device = VulkanDevice::Get();
		VkDevice vkDevice = device.GetDevice();
		if (vkDevice == VK_NULL_HANDLE)
			return;

		vkDeviceWaitIdle(vkDevice);

		m_TrianglePass.Shutdown();

		for (auto semaphore : m_RenderFinishedSemaphores)
			vkDestroySemaphore(vkDevice, semaphore, nullptr);
		for (size_t i = 0; i < MaxFramesInFlight; ++i)
		{
			vkDestroySemaphore(vkDevice, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(vkDevice, m_InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(vkDevice, m_CommandPool, nullptr);
		CleanupSwapchain();

		if (m_RenderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(vkDevice, m_RenderPass, nullptr);

		if (m_Surface != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(device.GetInstance(), m_Surface, nullptr);

		VulkanDevice::Get().Shutdown();
		s_Instance = nullptr;
	}

	void VulkanContext::Init()
	{
		auto& device = VulkanDevice::Get();
		if (!device.IsInitialized())
			device.InitInstance();

		CreateSurface();
		device.CreateDevice(m_Surface);

		CreateSwapchain();
		CreateImageViews();
		CreateRenderPass();
		CreateFramebuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSyncObjects();

		m_TrianglePass.Init(m_RenderPass);

		FE_CORE_INFO("Vulkan swapchain: {0}x{1}", m_SwapchainExtent.width, m_SwapchainExtent.height);
	}

	void VulkanContext::CreateSurface()
	{
		CheckVk(glfwCreateWindowSurface(VulkanDevice::Get().GetInstance(), m_WindowHandle, nullptr, &m_Surface),
			"glfwCreateWindowSurface");
	}

	VkSurfaceFormatKHR VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}
		return formats[0];
	}

	VkPresentModeKHR VulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes)
	{
		for (const auto& mode : modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		int width, height;
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);

		VkExtent2D extent;
		extent.width = (uint32_t)std::clamp(width, (int)capabilities.minImageExtent.width, (int)capabilities.maxImageExtent.width);
		extent.height = (uint32_t)std::clamp(height, (int)capabilities.minImageExtent.height, (int)capabilities.maxImageExtent.height);
		return extent;
	}

	void VulkanContext::CreateSwapchain()
	{
		auto& vulkanDevice = VulkanDevice::Get();
		VkPhysicalDevice physical = vulkanDevice.GetPhysicalDevice();

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, m_Surface, &capabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical, m_Surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical, m_Surface, &formatCount, formats.data());

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical, m_Surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical, m_Surface, &presentModeCount, presentModes.data());

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(presentModes);
		VkExtent2D extent = ChooseSwapExtent(capabilities);

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = {
			vulkanDevice.GetGraphicsQueueFamily(),
			vulkanDevice.GetPresentQueueFamily()
		};

		if (vulkanDevice.GetGraphicsQueueFamily() != vulkanDevice.GetPresentQueueFamily())
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		CheckVk(vkCreateSwapchainKHR(vulkanDevice.GetDevice(), &createInfo, nullptr, &m_Swapchain), "vkCreateSwapchainKHR");

		vkGetSwapchainImagesKHR(vulkanDevice.GetDevice(), m_Swapchain, &imageCount, nullptr);
		m_SwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(vulkanDevice.GetDevice(), m_Swapchain, &imageCount, m_SwapchainImages.data());

		m_SwapchainImageFormat = surfaceFormat.format;
		m_SwapchainExtent = extent;
		m_ImagesInFlight.resize(m_SwapchainImages.size(), VK_NULL_HANDLE);
	}

	void VulkanContext::CreateImageViews()
	{
		m_SwapchainImageViews.resize(m_SwapchainImages.size());
		auto& vulkanDevice = VulkanDevice::Get();

		for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapchainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			CheckVk(vkCreateImageView(vulkanDevice.GetDevice(), &createInfo, nullptr, &m_SwapchainImageViews[i]),
				"vkCreateImageView");
		}
	}

	void VulkanContext::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapchainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		CheckVk(vkCreateRenderPass(VulkanDevice::Get().GetDevice(), &renderPassInfo, nullptr, &m_RenderPass),
			"vkCreateRenderPass");
	}

	void VulkanContext::CreateFramebuffers()
	{
		auto& vulkanDevice = VulkanDevice::Get();
		m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());

		for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
		{
			VkImageView attachments[] = { m_SwapchainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapchainExtent.width;
			framebufferInfo.height = m_SwapchainExtent.height;
			framebufferInfo.layers = 1;

			CheckVk(vkCreateFramebuffer(vulkanDevice.GetDevice(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]),
				"vkCreateFramebuffer");
		}
	}

	void VulkanContext::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = VulkanDevice::Get().GetGraphicsQueueFamily();

		CheckVk(vkCreateCommandPool(VulkanDevice::Get().GetDevice(), &poolInfo, nullptr, &m_CommandPool), "vkCreateCommandPool");
	}

	void VulkanContext::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(MaxFramesInFlight);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		CheckVk(vkAllocateCommandBuffers(VulkanDevice::Get().GetDevice(), &allocInfo, m_CommandBuffers.data()),
			"vkAllocateCommandBuffers");
	}

	void VulkanContext::CreateSyncObjects()
	{
		auto& vulkanDevice = VulkanDevice::Get();
		m_ImageAvailableSemaphores.resize(MaxFramesInFlight);
		m_RenderFinishedSemaphores.resize(m_SwapchainImages.size());
		m_InFlightFences.resize(MaxFramesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); ++i)
		{
			CheckVk(vkCreateSemaphore(vulkanDevice.GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]),
				"vkCreateSemaphore");
		}

		for (int i = 0; i < MaxFramesInFlight; ++i)
		{
			CheckVk(vkCreateSemaphore(vulkanDevice.GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]),
				"vkCreateSemaphore");
			CheckVk(vkCreateFence(vulkanDevice.GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]), "vkCreateFence");
		}
	}

	void VulkanContext::CleanupSwapchain()
	{
		auto& vulkanDevice = VulkanDevice::Get();
		VkDevice device = vulkanDevice.GetDevice();
		if (device == VK_NULL_HANDLE)
			return;

		for (auto framebuffer : m_SwapchainFramebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		m_SwapchainFramebuffers.clear();

		for (auto imageView : m_SwapchainImageViews)
			vkDestroyImageView(device, imageView, nullptr);
		m_SwapchainImageViews.clear();

		if (m_Swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
			m_Swapchain = VK_NULL_HANDLE;
		}
	}

	void VulkanContext::RecreateSwapchain()
	{
		auto& vulkanDevice = VulkanDevice::Get();
		VkDevice device = vulkanDevice.GetDevice();
		vkDeviceWaitIdle(device);

		int width = 0, height = 0;
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_WindowHandle, &width, &height);
			glfwWaitEvents();
		}

		for (auto& semaphore : m_RenderFinishedSemaphores)
			vkDestroySemaphore(device, semaphore, nullptr);
		m_RenderFinishedSemaphores.clear();
		m_ImagesInFlight.clear();

		CleanupSwapchain();
		CreateSwapchain();
		CreateImageViews();
		CreateFramebuffers();

		m_RenderFinishedSemaphores.resize(m_SwapchainImages.size());
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); ++i)
		{
			CheckVk(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]),
				"vkCreateSemaphore");
		}

		m_FramebufferResized = false;
	}

	void VulkanContext::SwapBuffers()
	{
		auto& vulkanDevice = VulkanDevice::Get();
		VkDevice device = vulkanDevice.GetDevice();

		vkWaitForFences(device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex = 0;
		VkResult acquireResult = vkAcquireNextImageKHR(
			device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
		{
			RecreateSwapchain();
			return;
		}
		CheckVk(acquireResult, "vkAcquireNextImageKHR");

		if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(device, 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

		vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]);
		m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

		VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		CheckVk(vkBeginCommandBuffer(cmd, &beginInfo), "vkBeginCommandBuffer");

		VkClearValue clearColor{};
		clearColor.color = { { m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a } };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_SwapchainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapchainExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		if (m_DrawTriangle)
			m_TrianglePass.Record(cmd, m_SwapchainExtent);
		vkCmdEndRenderPass(cmd);

		CheckVk(vkEndCommandBuffer(cmd), "vkEndCommandBuffer");

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[imageIndex] };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		CheckVk(vkQueueSubmit(vulkanDevice.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]),
			"vkQueueSubmit");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.pImageIndices = &imageIndex;

		VkResult presentResult = vkQueuePresentKHR(vulkanDevice.GetPresentQueue(), &presentInfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
		{
			RecreateSwapchain();
		}
		else
		{
			CheckVk(presentResult, "vkQueuePresentKHR");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MaxFramesInFlight;
	}
}

#endif
