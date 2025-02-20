#include "sopch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"



namespace SpaceOptima
{
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;
	

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glGenerateTextureMipmap(m_RendererID);

		m_Handle = glGetTextureHandleARB(m_RendererID);

		
	}


	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		:m_Path(path)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		SO_CORE_ASSERT(data, "Failed to load image");
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

		SO_CORE_ASSERT(internalFormat & dataFormat, "Texture format not supported !");

		SO_CORE_TRACE("internal format = {0}, data format = {1}", internalFormat, dataFormat);

		m_DataFormat = dataFormat;

		m_InternalFormat = internalFormat;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);
		
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
		
		glGenerateTextureMipmap(m_RendererID);

		m_Handle = glGetTextureHandleARB(m_RendererID);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glMakeTextureHandleNonResidentARB(m_Handle);
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, size_t size) const
	{
		size_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		SO_CORE_ASSERT(size == m_Width * m_Height * bpp, "Size is not right !!!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::MakeTextureResident() const
	{
		if(!glIsImageHandleResidentARB(m_Handle))
			glMakeTextureHandleResidentARB(m_Handle);
	}

}
