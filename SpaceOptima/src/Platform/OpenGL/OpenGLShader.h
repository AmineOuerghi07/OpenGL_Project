#pragma once

#include <glm/glm.hpp>
#include "SpaceOptima/Renderer/Shader.h"

//TODO remove
typedef unsigned int GLenum;

namespace SpaceOptima
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name,const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader();
		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetMat3(const std::string& name, const glm::mat3& mat) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& mat) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& vec) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& vec) override;

		virtual const std::string& GetName() const override { return m_Name; }

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	


		void UploadUniformFloat(const std::string& name, const float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vec);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vec);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vec);

		void UploadUniformInt(const std::string& name, const int value);
		
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	private:
		uint32_t m_RendererID;
		std::string m_Name;
	};
}
