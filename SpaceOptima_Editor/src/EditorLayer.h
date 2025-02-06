#pragma once

#include <SpaceOptima.h>
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "SpaceOptima/Renderer/EditorCamera.h"

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
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMousePressed(MouseButtonPressedEvent& e);
		void NewScene();
		void SaveSceneAs();
		void OpenScene(const std::filesystem::path& path);
		void OpenScene();

	private:

		SpaceOptima::OrthographicCameraController m_CameraController;


		// Temporary
		/*Ref<Texture2D> m_Texture;
		Ref<Texture2D> m_TextureMilk;
		Ref<VertexArray> m_VertexArray;
		Ref<Shader> m_Shader;
		Ref<VertexArray> m_SquareVA;*/
		//Entity m_Square;
		
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;


	

		glm::vec2 m_ViewportSize = {1280.0, 720.0};

		glm::vec2 m_ViewportBounds[2];

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		Entity m_PrimaryCamera;
		Entity m_SecondCamera;

		Entity m_HoveredEntity;

		EditorCamera m_EditorCamera;

		bool m_IsPrimaryCamera = true;

		int m_GizmoType = -1;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		
		glm::vec4 m_SquareColor = { 0.8f, 0.0f, 0.1f, 1.0f };

	};
}