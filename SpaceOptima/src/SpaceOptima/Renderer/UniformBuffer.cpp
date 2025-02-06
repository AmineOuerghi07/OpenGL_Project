#include "sopch.h"
#include "UniformBuffer.h"
#include "SpaceOptima/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
namespace SpaceOptima {
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    SO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLUniformBuffer>(size, binding);
		}
		SO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}