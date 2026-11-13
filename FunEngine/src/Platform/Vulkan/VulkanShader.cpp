#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanShader.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanUtils.h"

namespace FunEngine
{
	namespace
	{
		static const VulkanShader* s_CurrentShader = nullptr;
		static uint64_t s_NextShaderID = 1;

		static std::string StripDirectoryAndExtension(const std::string& path)
		{
			size_t slash = path.find_last_of("/\\");
			std::string file = slash == std::string::npos ? path : path.substr(slash + 1);
			for (const char* suffix : { ".vert.spv", ".frag.spv", ".glsl", ".spv" })
			{
				const std::string s(suffix);
				if (file.size() > s.size() && file.compare(file.size() - s.size(), s.size(), s) == 0)
					return file.substr(0, file.size() - s.size());
			}
			return file;
		}
	}

	VulkanShader::VulkanShader(const std::string& filepath)
		: m_ID(s_NextShaderID++)
	{
		m_Name = StripDirectoryAndExtension(filepath);
		m_VertexModule = VulkanUtils::LoadShaderModule(m_Name + ".vert.spv");
		m_FragmentModule = VulkanUtils::LoadShaderModule(m_Name + ".frag.spv");
	}

	VulkanShader::VulkanShader(const std::string& name, const std::string&, const std::string&)
		: m_Name(name), m_ID(s_NextShaderID++)
	{
		FE_CORE_ERROR("Vulkan: shader '{0}' from GLSL source strings is not supported; provide precompiled SPIR-V", name);
	}

	VulkanShader::~VulkanShader()
	{
		VkDevice device = VulkanDevice::Get().GetDevice();
		if (s_CurrentShader == this)
			s_CurrentShader = nullptr;
		if (device == VK_NULL_HANDLE)
			return;

		VulkanPipelineCache::Get().InvalidateShader(m_ID);

		VkShaderModule vert = m_VertexModule;
		VkShaderModule frag = m_FragmentModule;
		auto destroy = [vert, frag]()
		{
			VkDevice d = VulkanDevice::Get().GetDevice();
			if (d == VK_NULL_HANDLE)
				return;
			if (vert != VK_NULL_HANDLE)
				vkDestroyShaderModule(d, vert, nullptr);
			if (frag != VK_NULL_HANDLE)
				vkDestroyShaderModule(d, frag, nullptr);
		};
		if (auto* ctx = VulkanContext::Get())
			ctx->DeferDestroy(destroy);
		else
			destroy();
	}

	void VulkanShader::Bind() const
	{
		s_CurrentShader = this;
	}

	void VulkanShader::Unbind() const
	{
		if (s_CurrentShader == this)
			s_CurrentShader = nullptr;
	}

	const VulkanShader* VulkanShader::GetCurrent()
	{
		return s_CurrentShader;
	}

	void VulkanShader::WarnUniformStub(const char* what) const
	{
		static bool warned = false;
		if (!warned)
		{
			FE_CORE_WARN("Vulkan: Shader::{0} is a no-op (uniforms come from UniformBuffer / texture descriptors)", what);
			warned = true;
		}
	}

	void VulkanShader::SetInt(const std::string&, int) { WarnUniformStub("SetInt"); }
	void VulkanShader::SetIntArray(const std::string&, int*, uint32_t) { /* texture slots are implicit */ }
	void VulkanShader::SetFloat(const std::string&, float) { WarnUniformStub("SetFloat"); }
	void VulkanShader::SetFloat2(const std::string&, const glm::vec2&) { WarnUniformStub("SetFloat2"); }
	void VulkanShader::SetFloat3(const std::string&, const glm::vec3&) { WarnUniformStub("SetFloat3"); }
	void VulkanShader::SetFloat4(const std::string&, const glm::vec4&) { WarnUniformStub("SetFloat4"); }
	void VulkanShader::SetMat4(const std::string&, const glm::mat4&) { WarnUniformStub("SetMat4"); }
	void VulkanShader::SetMat3(const std::string&, const glm::mat3&) { WarnUniformStub("SetMat3"); }
}

#endif
