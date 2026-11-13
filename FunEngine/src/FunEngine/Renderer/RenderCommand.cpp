#include "fepch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanRendererAPI.h"
#endif

namespace FunEngine
{
	RendererAPI* RenderCommand::s_RendererAPI = nullptr;

	void RenderCommand::EnsureRendererAPI()
	{
		if (s_RendererAPI)
			return;

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			s_RendererAPI = new OpenGLRendererAPI();
			break;
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan:
			s_RendererAPI = new VulkanRendererAPI();
			break;
#endif
		default:
			FE_CORE_ASSERT(false, "RenderCommand: unsupported RendererAPI");
			s_RendererAPI = new OpenGLRendererAPI();
			break;
		}
	}
}
