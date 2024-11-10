#pragma once

#include <SpaceOptima.h>

namespace SpaceOptima
{
	class EditorLayer : public SpaceOptima::Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(SpaceOptima::Timestep ts) override;
		void OnImGuiRender() override;
		void OnEvent(SpaceOptima::Event& e) override;

	private:

		SpaceOptima::OrthographicCameraController m_CameraController;


		// Temporary
		Ref<Texture2D> m_Texture;
		Ref<Texture2D> m_TextureMilk;
		Ref<VertexArray> m_VertexArray;
		Ref<Shader> m_Shader;
		Ref<VertexArray> m_SquareVA;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;

		Entity m_Square;

		glm::vec2 m_ViewportSize = {1280.0, 720.0};

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;


		glm::vec4 m_SquareColor = { 0.8f, 0.0f, 0.1f, 1.0f };

	};
}