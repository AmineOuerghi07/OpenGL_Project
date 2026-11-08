#include "fepch.h"

#include "Platform/OpenGL/OpenGLStorageBuffer.h"

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
			}
			FE_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}

}