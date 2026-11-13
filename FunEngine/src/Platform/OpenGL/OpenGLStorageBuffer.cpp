#include "fepch.h"
#include "OpenGLStorageBuffer.h"
#include <glad/glad.h>
namespace FunEngine {
	OpenGLStorageBuffer::OpenGLStorageBuffer(uint64_t size, uint32_t binding)
	{
#ifdef FE_PLATFORM_MACOS
		// SSBOs require GL 4.3+; Apple OpenGL tops out at ~4.1.
		// Create a plain buffer so the type still links; callers must not rely on SSBO on Mac.
		(void)binding;
		m_RendererID = 0;
		FE_CORE_WARN("StorageBuffer is unavailable on macOS OpenGL 4.1 (requested size {0})", size);
#else
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
#endif
	}
	OpenGLStorageBuffer::~OpenGLStorageBuffer()
	{
#ifndef FE_PLATFORM_MACOS
		glDeleteBuffers(1, &m_RendererID);
#endif
	}
	void OpenGLStorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
#ifdef FE_PLATFORM_MACOS
		(void)data; (void)size; (void)offset;
#else
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
#endif
	}
}
