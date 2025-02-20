#include "sopch.h"

#include "Platform/OpenGL/OpenGLStorageBuffer.h"

#include "SpaceOptima/Renderer/Renderer.h"

#include "StorageBuffer.h"

namespace SpaceOptima
{

		Ref<StorageBuffer> StorageBuffer::Create(uint64_t size, uint32_t binding)
		{
			switch (Renderer::GetAPI())
			{
			case RendererAPI::API::None:    SO_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLStorageBuffer>(size, binding);
			}
			SO_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}

}