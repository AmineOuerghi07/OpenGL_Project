#include "fepch.h"
#include "Buffer.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanBuffers.h"
#endif

namespace FunEngine
{

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported"); break;
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexBuffer>(vertices, size); break;
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan: return std::make_shared<VulkanVertexBuffer>(vertices, size); break;
#endif
		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported"); break;
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexBuffer>(size); break;
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan: return std::make_shared<VulkanVertexBuffer>(size); break;
#endif
		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;

	}


	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported"); break;
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLIndexBuffer>(indices, size); break;
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan: return std::make_shared<VulkanIndexBuffer>(indices, size); break;
#endif

		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;

	}
}