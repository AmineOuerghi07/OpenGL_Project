#pragma once

#ifdef FE_WITH_VULKAN

#include "FunEngine/Renderer/GraphicsContext.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include "VulkanTrianglePass.h"

struct GLFWwindow;

namespace FunEngine
{
	class VulkanContext : public GraphicsContext
	{
	public:
		static constexpr int MaxFramesInFlight = 2;

		static VulkanContext* Get() { return s_Instance; }

		VulkanContext(GLFWwindow* windowHandle);
		~VulkanContext() override;

		void Init() override;
		void SwapBuffers() override;

		void SetFramebufferResized(bool resized) { m_FramebufferResized = resized; }
		void SetClearColor(const glm::vec4& color) { m_ClearColor = color; }
		const glm::vec4& GetClearColor() const { return m_ClearColor; }

		VkExtent2D GetExtent() const { return m_SwapchainExtent; }
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkCommandBuffer GetCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentFrame]; }

	private:
		void CreateSurface();
		void CreateSwapchain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		void CleanupSwapchain();
		void RecreateSwapchain();
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		static VulkanContext* s_Instance;

		GLFWwindow* m_WindowHandle = nullptr;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> m_SwapchainImages;
		VkFormat m_SwapchainImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_SwapchainExtent{};
		std::vector<VkImageView> m_SwapchainImageViews;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores; // per frame in flight
		std::vector<VkSemaphore> m_RenderFinishedSemaphores; // per swapchain image
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		VulkanTrianglePass m_TrianglePass;
		bool m_DrawTriangle = true;

		uint32_t m_CurrentFrame = 0;
		bool m_FramebufferResized = false;
		glm::vec4 m_ClearColor{ 0.08f, 0.08f, 0.18f, 1.0f };
	};
}

#endif
