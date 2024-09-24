#include "sopch.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
namespace SpaceOptima
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None: SO_CORE_ASSERT(false, "RendererAPI: None is currently not supported");
		case RendererAPI::OpenGL: return new OpenGLVertexArray();
		}

		SO_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}