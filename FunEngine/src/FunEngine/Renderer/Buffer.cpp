#include "fepch.h"
#include "Buffer.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace FunEngine
{

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: FE_CORE_ASSERT(false, "RendererAPI: None is currently not supported"); break;
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexBuffer>(size); break;
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
		
		}

		FE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;

	}
}