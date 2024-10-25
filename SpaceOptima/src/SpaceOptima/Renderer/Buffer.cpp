#include "sopch.h"
#include "Buffer.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace SpaceOptima
{

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: SO_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexBuffer>(size);
		}

		SO_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;

	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				SO_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		
			case RendererAPI::API::OpenGL: 
				return std::make_shared<OpenGLVertexBuffer>(vertices, size); 
		}

		SO_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;

	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: SO_CORE_ASSERT(false, "RendererAPI: None is currently not supported"); 
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLIndexBuffer>(indices, size);
		}

		SO_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;

	}
}