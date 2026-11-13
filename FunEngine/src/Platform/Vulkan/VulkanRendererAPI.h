#pragma once

#ifdef FE_WITH_VULKAN

#include "FunEngine/Renderer/RendererAPI.h"

namespace FunEngine
{
	class VulkanRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

	private:
		glm::vec4 m_ClearColor{ 0.08f, 0.08f, 0.18f, 1.0f };
	};
}

#endif
