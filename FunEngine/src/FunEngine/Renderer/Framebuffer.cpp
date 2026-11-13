#include "fepch.h"
#include "Framebuffer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"
#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanFramebuffer.h"
#endif
#include "Renderer.h"


namespace FunEngine
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLFramebuffer>(spec);
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan: return std::make_shared<VulkanFramebuffer>(spec);
#endif
		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}