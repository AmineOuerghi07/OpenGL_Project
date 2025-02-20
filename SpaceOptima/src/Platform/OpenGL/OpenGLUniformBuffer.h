#pragma once
#include "SpaceOptima/Renderer/UniformBuffer.h"
namespace SpaceOptima {
	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding, uint32_t renderer = 0);
		virtual ~OpenGLUniformBuffer();
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		uint32_t m_RendererID = 0;
	};
}