#pragma once

#include "SpaceOptima/Renderer/Texture.h"
#include <glad/glad.h>

namespace SpaceOptima
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; };
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void Bind(uint32_t slot = 0) const override;
		virtual void SetData(void* data, size_t size) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_RendererID;
		GLenum m_DataFormat;
		GLenum m_InternalFormat;
	};
}