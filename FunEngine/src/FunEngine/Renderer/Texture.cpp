#include "fepch.h"

#include "Texture.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanTexture.h"
#endif

#include "FunEngine/Renderer/Renderer.h"

namespace FunEngine
{
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan: return std::make_shared<VulkanTexture2D>(path);
#endif
		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(width, height);
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan: return std::make_shared<VulkanTexture2D>(width, height);
#endif
		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}