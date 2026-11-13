#include "fepch.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanBuffers.h"
#endif
namespace FunEngine
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexArray>();
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan: return std::make_shared<VulkanVertexArray>();
#endif
		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}