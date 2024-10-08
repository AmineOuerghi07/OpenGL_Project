#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "SpaceOptima/Renderer/VertexArray.h"
#include "SpaceOptima/Renderer/RendererAPI.h"

namespace SpaceOptima
{
	class RenderCommand
	{
	public:

		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		 static RendererAPI* s_RendererAPI;
	};
}