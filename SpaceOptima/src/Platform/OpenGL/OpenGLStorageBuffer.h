#pragma once
#include "SpaceOptima/Renderer/StorageBuffer.h"
namespace SpaceOptima {
	class OpenGLStorageBuffer : public StorageBuffer
	{
	public:
		OpenGLStorageBuffer(uint64_t size, uint32_t binding);
		virtual ~OpenGLStorageBuffer();
		virtual void SetData(const void* data, uint32_t size, uint32_t offset) override;
	private:
		uint32_t m_RendererID;
	};
}