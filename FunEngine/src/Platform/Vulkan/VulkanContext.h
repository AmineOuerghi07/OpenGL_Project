#pragma once

#ifdef FE_WITH_VULKAN

#include "FunEngine/Renderer/GraphicsContext.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include <functional>
#include "VulkanTrianglePass.h"
#include "VulkanPipeline.h"

struct GLFWwindow;

namespace FunEngine
{
	class VulkanFramebuffer;

	class VulkanContext : public GraphicsContext
	{
	public:
		static constexpr int MaxFramesInFlight = 2;

		static VulkanContext* Get() { return s_Instance; }
		static bool IsImGuiInitialized() { return s_ImGuiInitialized; }
		static void SetImGuiInitialized(bool initialized) { s_ImGuiInitialized = initialized; }

		VulkanContext(GLFWwindow* windowHandle);
		~VulkanContext() override;

		void Init() override;

		// Frame: BeginFrame acquires the image and starts recording; draws record lazily into the
		// current render target; SwapBuffers (EndFrame) closes the pass, submits and presents.
		bool BeginFrame();
		void SwapBuffers() override;
		void EndFrame() { SwapBuffers(); }
		bool IsFrameActive() const { return m_FrameActive; }
		void WaitIdle();

		uint32_t GetFrameIndex() const { return m_CurrentFrame; }
		uint64_t GetFrameSerial() const { return m_FrameSerial; }
		uint32_t GetSwapchainImageCount() const { return (uint32_t)m_SwapchainImages.size(); }

		// Runs once the GPU has finished the frame slot that was current when this was queued.
		void DeferDestroy(std::function<void()> destroyFn);

		void SetCurrentFramebuffer(VulkanFramebuffer* framebuffer);
		VulkanFramebuffer* GetCurrentFramebuffer() const { return m_CurrentFramebuffer; }
		VulkanTargetInfo GetCurrentTargetInfo() const;

		// Begins the render pass of the current target (swapchain or framebuffer) if not active.
		bool EnsureRenderPass();
		void ClearCurrentTarget();

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
		void BeginSwapchainPass(bool clear);
		void EndSwapchainPass();
		void RunDeferredDestroys(uint32_t slot);
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
		static bool s_ImGuiInitialized;

		GLFWwindow* m_WindowHandle = nullptr;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> m_SwapchainImages;
		VkFormat m_SwapchainImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D m_SwapchainExtent{};
		std::vector<VkImageView> m_SwapchainImageViews;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkRenderPass m_RenderPassLoad = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores; // per frame in flight
		std::vector<VkSemaphore> m_RenderFinishedSemaphores; // per swapchain image
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		VulkanTrianglePass m_TrianglePass;
		bool m_DrawTriangle = false;

		bool m_FrameActive = false;
		uint32_t m_ImageIndex = 0;
		uint64_t m_FrameSerial = 0;
		bool m_SwapchainPassActive = false;
		bool m_SwapchainPassUsed = false;
		VulkanFramebuffer* m_CurrentFramebuffer = nullptr;
		std::vector<std::function<void()>> m_DeferredDestroys[MaxFramesInFlight];

		uint32_t m_CurrentFrame = 0;
		bool m_FramebufferResized = false;
		glm::vec4 m_ClearColor{ 0.08f, 0.08f, 0.18f, 1.0f };
	};
}

#endif
