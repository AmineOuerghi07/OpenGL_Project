#pragma once

#ifdef FE_WITH_VULKAN

#include "FunEngine/Renderer/Shader.h"
#include <vulkan/vulkan.h>

namespace FunEngine
{
	// Loads a precompiled SPIR-V pair. Accepted paths:
	//   "Renderer2D"                       -> Renderer2D.vert.spv + Renderer2D.frag.spv
	//   "assets/shaders/Texture.glsl"      -> Texture.vert.spv + Texture.frag.spv (GL path stem reused)
	//   "foo.vert.spv" / "foo.frag.spv"    -> the matching pair
	// Files are resolved from assets/shaders/vulkan/ (or the shared FunEngine/assets tree).
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& filepath);
		VulkanShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~VulkanShader() override;

		void Bind() const override;
		void Unbind() const override;

		void SetInt(const std::string& name, int value) override;
		void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		void SetFloat(const std::string& name, float value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetMat4(const std::string& name, const glm::mat4& value) override;
		void SetMat3(const std::string& name, const glm::mat3& mat) override;

		const std::string& GetName() const override { return m_Name; }

		VkShaderModule GetVertexModule() const { return m_VertexModule; }
		VkShaderModule GetFragmentModule() const { return m_FragmentModule; }
		bool IsValid() const { return m_VertexModule != VK_NULL_HANDLE && m_FragmentModule != VK_NULL_HANDLE; }
		uint64_t GetID() const { return m_ID; }

		static const VulkanShader* GetCurrent();

	private:
		void WarnUniformStub(const char* what) const;

		std::string m_Name;
		VkShaderModule m_VertexModule = VK_NULL_HANDLE;
		VkShaderModule m_FragmentModule = VK_NULL_HANDLE;
		uint64_t m_ID = 0;
	};
}

#endif
