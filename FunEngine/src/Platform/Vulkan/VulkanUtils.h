#pragma once

#ifdef FE_WITH_VULKAN

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <cstdint>

namespace FunEngine
{
	namespace VulkanUtils
	{
		void CheckVk(VkResult result, const char* where);

		bool ReadBinaryFile(const std::string& path, std::vector<uint32_t>& outWords);

		// Resolves a shader asset relative to the working directory, falling back to the
		// shared FunEngine/assets tree so Sandbox and Editor can both run from their own cwd.
		std::string ResolveShaderPath(const std::string& relativePath);

		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);
		VkShaderModule LoadShaderModule(const std::string& path);
	}
}

#endif
