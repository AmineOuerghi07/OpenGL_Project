#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanUtils.h"
#include "VulkanDevice.h"

#include <fstream>

namespace FunEngine
{
	namespace VulkanUtils
	{
		void CheckVk(VkResult result, const char* where)
		{
			if (result != VK_SUCCESS)
			{
				FE_CORE_ERROR("Vulkan error {0} at {1}", (int)result, where);
				FE_CORE_ASSERT(false, "Vulkan call failed");
			}
		}

		bool ReadBinaryFile(const std::string& path, std::vector<uint32_t>& outWords)
		{
			std::ifstream file(path, std::ios::ate | std::ios::binary);
			if (!file.is_open())
				return false;

			const size_t size = (size_t)file.tellg();
			if (size == 0 || size % 4 != 0)
				return false;

			outWords.resize(size / 4);
			file.seekg(0);
			file.read(reinterpret_cast<char*>(outWords.data()), (std::streamsize)size);
			return true;
		}

		std::string ResolveShaderPath(const std::string& relativePath)
		{
			static const char* prefixes[] = {
				"assets/shaders/vulkan/",
				"../FunEngine/assets/shaders/vulkan/",
				"FunEngine/assets/shaders/vulkan/",
				"../../FunEngine/assets/shaders/vulkan/"
			};

			for (const char* prefix : prefixes)
			{
				std::string candidate = std::string(prefix) + relativePath;
				std::ifstream file(candidate, std::ios::binary);
				if (file.good())
					return candidate;
			}
			return {};
		}

		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code)
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size() * sizeof(uint32_t);
			createInfo.pCode = code.data();

			VkShaderModule module = VK_NULL_HANDLE;
			CheckVk(vkCreateShaderModule(VulkanDevice::Get().GetDevice(), &createInfo, nullptr, &module), "vkCreateShaderModule");
			return module;
		}

		VkShaderModule LoadShaderModule(const std::string& relativePath)
		{
			std::string resolved = ResolveShaderPath(relativePath);
			std::vector<uint32_t> words;
			if (resolved.empty() || !ReadBinaryFile(resolved, words))
			{
				FE_CORE_ERROR("Vulkan: could not load SPIR-V shader '{0}' (run scripts/compile_vulkan_shaders.sh)", relativePath);
				FE_CORE_ASSERT(false, "Missing SPIR-V shader");
				return VK_NULL_HANDLE;
			}
			return CreateShaderModule(words);
		}
	}
}

#endif
