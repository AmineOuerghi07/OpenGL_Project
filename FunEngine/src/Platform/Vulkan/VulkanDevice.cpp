#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanDevice.h"

#include <GLFW/glfw3.h>
#include <cstring>
#include <set>

namespace FunEngine
{
	namespace
	{
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void*)
		{
			if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				FE_CORE_WARN("Vulkan validation: {0}", pCallbackData->pMessage);
			return VK_FALSE;
		}

		static VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func)
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		static void DestroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func)
				func(instance, debugMessenger, pAllocator);
		}

		static void CheckVk(VkResult result, const char* where)
		{
			if (result != VK_SUCCESS)
			{
				FE_CORE_ERROR("Vulkan error {0} at {1}", (int)result, where);
				FE_CORE_ASSERT(false, "Vulkan call failed");
			}
		}

		static bool HasDeviceExtension(VkPhysicalDevice device, const char* name)
		{
			uint32_t count = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
			std::vector<VkExtensionProperties> props(count);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &count, props.data());
			for (const auto& ext : props)
			{
				if (strcmp(ext.extensionName, name) == 0)
					return true;
			}
			return false;
		}
	}

	VulkanDevice& VulkanDevice::Get()
	{
		static VulkanDevice instance;
		return instance;
	}

	bool VulkanDevice::CheckValidationLayerSupport()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> available(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, available.data());
		for (const auto& layer : available)
		{
			if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
				return true;
		}
		return false;
	}

	void VulkanDevice::InitInstance()
	{
		if (m_InstanceReady)
			return;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef FE_PLATFORM_MACOS
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "FunEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		appInfo.pEngineName = "FunEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = (uint32_t)extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef FE_PLATFORM_MACOS
		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

		std::vector<const char*> layers;
		bool enableValidation = false;
#if defined(FE_ENGINE_DEBUG)
		if (CheckValidationLayerSupport())
		{
			layers.push_back("VK_LAYER_KHRONOS_validation");
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			enableValidation = true;
		}
		createInfo.enabledLayerCount = (uint32_t)layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = (uint32_t)extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();
#endif

		CheckVk(vkCreateInstance(&createInfo, nullptr, &m_Instance), "vkCreateInstance");

#if defined(FE_ENGINE_DEBUG)
		if (enableValidation)
			CreateDebugMessenger();
#endif

		m_InstanceReady = true;
		m_Initialized = true;
		FE_CORE_INFO("Vulkan instance created");
	}

	void VulkanDevice::CreateDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
		CheckVk(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger), "CreateDebugUtilsMessengerEXT");
	}

	bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int graphicsFamily = -1;
		int presentFamily = -1;
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphicsFamily = (int)i;
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				presentFamily = (int)i;
			if (graphicsFamily >= 0 && presentFamily >= 0)
				break;
		}

		if (graphicsFamily < 0 || presentFamily < 0)
			return false;

		if (!HasDeviceExtension(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			return false;

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);
		return features.samplerAnisotropy || true;
	}

	void VulkanDevice::PickPhysicalDevice(VkSurfaceKHR surface)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		FE_CORE_ASSERT(deviceCount > 0, "No Vulkan physical devices");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device, surface))
			{
				m_PhysicalDevice = device;
				VkPhysicalDeviceProperties props;
				vkGetPhysicalDeviceProperties(device, &props);
				FE_CORE_INFO("Vulkan physical device: {0}", props.deviceName);
				FE_CORE_INFO("Vulkan API version: {0}.{1}.{2}",
					VK_VERSION_MAJOR(props.apiVersion),
					VK_VERSION_MINOR(props.apiVersion),
					VK_VERSION_PATCH(props.apiVersion));
				return;
			}
		}
		FE_CORE_ASSERT(false, "No suitable Vulkan GPU");
	}

	void VulkanDevice::QueryDescriptorIndexingFeatures()
	{
		VkPhysicalDeviceDescriptorIndexingFeatures indexing{};
		indexing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

		VkPhysicalDeviceFeatures2 features2{};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &indexing;

		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);

		m_DescriptorIndexing.RuntimeDescriptorArray = indexing.runtimeDescriptorArray == VK_TRUE;
		m_DescriptorIndexing.DescriptorBindingPartiallyBound = indexing.descriptorBindingPartiallyBound == VK_TRUE;
		m_DescriptorIndexing.ShaderSampledImageArrayNonUniformIndexing =
			indexing.shaderSampledImageArrayNonUniformIndexing == VK_TRUE;
		m_DescriptorIndexing.DescriptorBindingSampledImageUpdateAfterBind =
			indexing.descriptorBindingSampledImageUpdateAfterBind == VK_TRUE;

		VkPhysicalDeviceDescriptorIndexingProperties indexingProps{};
		indexingProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
		VkPhysicalDeviceProperties2 props2{};
		props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props2.pNext = &indexingProps;
		vkGetPhysicalDeviceProperties2(m_PhysicalDevice, &props2);
		m_DescriptorIndexing.MaxPerStageUpdateAfterBindSampledImages = indexingProps.maxPerStageDescriptorUpdateAfterBindSampledImages;
		m_DescriptorIndexing.MaxPerStageUpdateAfterBindSamplers = indexingProps.maxPerStageDescriptorUpdateAfterBindSamplers;

		FE_CORE_INFO("Vulkan descriptor indexing: runtimeDescriptorArray={0}, partiallyBound={1}, nonUniformIndexing={2}, sampledImageUpdateAfterBind={3}",
			m_DescriptorIndexing.RuntimeDescriptorArray,
			m_DescriptorIndexing.DescriptorBindingPartiallyBound,
			m_DescriptorIndexing.ShaderSampledImageArrayNonUniformIndexing,
			m_DescriptorIndexing.DescriptorBindingSampledImageUpdateAfterBind);
	}

	void VulkanDevice::CreateLogicalDevice(VkSurfaceKHR surface)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		m_GraphicsQueueFamily = UINT32_MAX;
		m_PresentQueueFamily = UINT32_MAX;
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				m_GraphicsQueueFamily = i;
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, surface, &presentSupport);
			if (presentSupport)
				m_PresentQueueFamily = i;
		}
		FE_CORE_ASSERT(m_GraphicsQueueFamily != UINT32_MAX && m_PresentQueueFamily != UINT32_MAX, "Queue families");

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueFamilies = { m_GraphicsQueueFamily, m_PresentQueueFamily };
		float queuePriority = 1.0f;
		for (uint32_t family : uniqueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = family;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#ifdef FE_PLATFORM_MACOS
		if (HasDeviceExtension(m_PhysicalDevice, "VK_KHR_portability_subset"))
			m_DeviceExtensions.push_back("VK_KHR_portability_subset");
#endif

		QueryDescriptorIndexingFeatures();

		VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexingFeatures.runtimeDescriptorArray = m_DescriptorIndexing.RuntimeDescriptorArray;
		indexingFeatures.descriptorBindingPartiallyBound = m_DescriptorIndexing.DescriptorBindingPartiallyBound;
		indexingFeatures.shaderSampledImageArrayNonUniformIndexing = m_DescriptorIndexing.ShaderSampledImageArrayNonUniformIndexing;
		indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = m_DescriptorIndexing.DescriptorBindingSampledImageUpdateAfterBind;

		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &indexingFeatures;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = nullptr;
		createInfo.pNext = &deviceFeatures2;
		createInfo.enabledExtensionCount = (uint32_t)m_DeviceExtensions.size();
		createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

		CheckVk(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device), "vkCreateDevice");
		vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, m_PresentQueueFamily, 0, &m_PresentQueue);

		QueryDeviceLimits();
		m_DeviceReady = true;
	}

	void VulkanDevice::QueryDeviceLimits()
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);
		m_Limits = props.limits;

		const VkFormat candidates[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT };
		for (VkFormat format : candidates)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_DepthStencilFormat = format;
				break;
			}
		}
		FE_CORE_ASSERT(m_DepthStencilFormat != VK_FORMAT_UNDEFINED, "No depth/stencil format available");
	}

	uint32_t VulkanDevice::FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProps);
		for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
		{
			if ((typeBits & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		FE_CORE_ASSERT(false, "Failed to find suitable Vulkan memory type");
		return 0;
	}

	void VulkanDevice::CreateDevice(VkSurfaceKHR surface)
	{
		if (!m_InstanceReady)
			InitInstance();
		if (m_DeviceReady)
			return;

		PickPhysicalDevice(surface);
		CreateLogicalDevice(surface);
	}

	void VulkanDevice::Shutdown()
	{
		if (m_Device != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(m_Device);
			vkDestroyDevice(m_Device, nullptr);
			m_Device = VK_NULL_HANDLE;
		}
		m_DeviceReady = false;

#if defined(FE_ENGINE_DEBUG)
		if (m_DebugMessenger != VK_NULL_HANDLE)
		{
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
			m_DebugMessenger = VK_NULL_HANDLE;
		}
#endif

		if (m_Instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(m_Instance, nullptr);
			m_Instance = VK_NULL_HANDLE;
		}
		m_InstanceReady = false;
		m_Initialized = false;
	}
}

#endif
