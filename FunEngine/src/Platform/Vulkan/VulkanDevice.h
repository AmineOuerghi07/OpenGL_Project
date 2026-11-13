#pragma once

#ifdef FE_WITH_VULKAN

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

struct GLFWwindow;

namespace FunEngine
{
	struct VulkanDescriptorIndexingSupport
	{
		bool RuntimeDescriptorArray = false;
		bool DescriptorBindingPartiallyBound = false;
		bool ShaderSampledImageArrayNonUniformIndexing = false;
	};

	class VulkanDevice
	{
	public:
		static VulkanDevice& Get();

		void InitInstance();
		void CreateDevice(VkSurfaceKHR surface);
		void Shutdown();

		bool IsInitialized() const { return m_Initialized; }

		VkInstance GetInstance() const { return m_Instance; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice GetDevice() const { return m_Device; }
		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }
		uint32_t GetGraphicsQueueFamily() const { return m_GraphicsQueueFamily; }
		uint32_t GetPresentQueueFamily() const { return m_PresentQueueFamily; }

		const VulkanDescriptorIndexingSupport& GetDescriptorIndexingSupport() const { return m_DescriptorIndexing; }

	private:
		VulkanDevice() = default;
		~VulkanDevice() = default;

		bool CheckValidationLayerSupport();
		void CreateDebugMessenger();
		void PickPhysicalDevice(VkSurfaceKHR surface);
		void CreateLogicalDevice(VkSurfaceKHR surface);
		void QueryDescriptorIndexingFeatures();
		bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

		bool m_Initialized = false;
		bool m_InstanceReady = false;
		bool m_DeviceReady = false;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
		uint32_t m_GraphicsQueueFamily = UINT32_MAX;
		uint32_t m_PresentQueueFamily = UINT32_MAX;

		VulkanDescriptorIndexingSupport m_DescriptorIndexing;
		std::vector<const char*> m_DeviceExtensions;
	};
}

#endif
