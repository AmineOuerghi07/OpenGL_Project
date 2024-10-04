#include "sopch.h"

#include "Texture.h"

#include "Platform/OpenGL/OpenGLTexture.h"

#include "SpaceOptima/Renderer/Renderer.h"

namespace SpaceOptima
{
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: SO_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
		}

		SO_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}