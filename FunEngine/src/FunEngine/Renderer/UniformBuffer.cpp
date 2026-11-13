#include "fepch.h"
#include "UniformBuffer.h"
#include "FunEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanBuffers.h"
#endif
namespace FunEngine {
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    FE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLUniformBuffer>(size, binding);
#ifdef FE_WITH_VULKAN
		case RendererAPI::API::Vulkan:  return std::make_shared<VulkanUniformBuffer>(size, binding);
#endif
		}
		FE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}