#include "fepch.h"

#include "Platform/OpenGL/OpenGLStorageBuffer.h"
#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanBuffers.h"
#endif

#include "FunEngine/Renderer/Renderer.h"

#include "StorageBuffer.h"

namespace FunEngine
{

		Ref<StorageBuffer> StorageBuffer::Create(uint64_t size, uint32_t binding)
		{
			switch (Renderer::GetAPI())
			{
			case RendererAPI::API::None:    FE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLStorageBuffer>(size, binding);
#ifdef FE_WITH_VULKAN
			case RendererAPI::API::Vulkan:  return std::make_shared<VulkanStorageBuffer>(size, binding);
#endif
			}
			FE_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}

}