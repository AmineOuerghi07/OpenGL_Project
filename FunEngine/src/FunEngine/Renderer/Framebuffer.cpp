#include "fepch.h"
#include "Framebuffer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Renderer.h"


namespace FunEngine
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLFramebuffer>(spec);
		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}