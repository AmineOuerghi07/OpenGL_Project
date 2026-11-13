#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "FunEngine/Renderer/VertexArray.h"
#include "FunEngine/Renderer/RendererAPI.h"

namespace FunEngine
{
	class RenderCommand
	{
	public:

		inline static void Init()
		{
			EnsureRendererAPI();
			s_RendererAPI->Init();
		}

		inline static void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			EnsureRendererAPI();
			s_RendererAPI->SetViewPort(x, y, width, height);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			EnsureRendererAPI();
			s_RendererAPI->SetClearColor(color);
		}
		inline static void Clear()
		{
			EnsureRendererAPI();
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
		{
			EnsureRendererAPI();
			s_RendererAPI->DrawIndexed(vertexArray, count);
		}
	private:
		static void EnsureRendererAPI();
		static RendererAPI* s_RendererAPI;
	};
}