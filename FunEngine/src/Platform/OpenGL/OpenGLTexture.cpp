#include "fepch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"



namespace FunEngine
{
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;
		m_Handle = 0;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#ifndef FE_PLATFORM_MACOS
		glGenerateMipmap(GL_TEXTURE_2D);
		m_Handle = glGetTextureHandleARB(m_RendererID);
		glMakeTextureHandleResidentARB(m_Handle);
#endif
	}


	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		:m_Path(path), m_Handle(0)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		FE_CORE_ASSERT(data, "Failed to load image");
		m_Height = height;
		m_Width = width;


		GLenum internalFormat = 0, dataFormat = 0;

		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		FE_CORE_ASSERT(internalFormat & dataFormat, "Texture format not supported !");

		FE_CORE_TRACE("internal format = {0}, data format = {1}", internalFormat, dataFormat);

		m_DataFormat = dataFormat;

		m_InternalFormat = internalFormat;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenerateMipmap(GL_TEXTURE_2D);

#ifndef FE_PLATFORM_MACOS
		m_Handle = glGetTextureHandleARB(m_RendererID);
		glMakeTextureHandleResidentARB(m_Handle);
#endif

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
#ifndef FE_PLATFORM_MACOS
		if (m_Handle)
			glMakeTextureHandleNonResidentARB(m_Handle);
#endif
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, size_t size) const
	{
		size_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		FE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Size is not right !!!");
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}


}
