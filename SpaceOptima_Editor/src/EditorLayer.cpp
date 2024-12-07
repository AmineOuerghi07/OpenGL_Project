#include "EditorLayer.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLShader.h"
#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include "SpaceOptima/Scene/SceneSerializer.h"

#include "SpaceOptima/Utils/PlatformUtils.h"

#include "SpaceOptima/Math/Math.h"

#include "ImGuizmo.h"

namespace SpaceOptima
{

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f, false)
	{
	}

	void EditorLayer::OnAttach()
	{

		m_Texture = Texture2D::Create("assets/textures/textureTest.jpg");
		m_TextureMilk = Texture2D::Create("assets/textures/test.jpg");

		FramebufferSpecification fbSpecs;

		fbSpecs.Attachments = { FramebufferTextureFormat::RGBA8,FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };

		fbSpecs.Height = 720;
		fbSpecs.Width = 1280;

		m_Framebuffer = Framebuffer::Create(fbSpecs);

		m_ActiveScene = std::make_shared<Scene>();

		m_EditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
#if 0
		auto square = m_ActiveScene->CreateEntity("Square");


		square.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f,1.0f,0.0f,1.0f });

		auto blueSquare = m_ActiveScene->CreateEntity("Square");


		blueSquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f,0.0f,1.0f,1.0f });

		m_Square = square;

		m_PrimaryCamera = m_ActiveScene->CreateEntity("Primary Camera");

		m_PrimaryCamera.AddComponent<CameraComponent>();

		m_SecondCamera = m_ActiveScene->CreateEntity("Second Camera");

		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();

		cc.Primary = false;
#endif
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		//SceneSerializer sceneSerializer = SceneSerializer(m_ActiveScene);
		//sceneSerializer.Serialize("assets/scenes/exampleScene.so");
		//sceneSerializer.Deserialize("assets/scenes/exampleScene.so");


	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		// Update
		// Reset here


		// Resize

		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize(uint32_t(m_ViewportSize.x), uint32_t(m_ViewportSize.y));
			
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->OnViewportResize(uint32_t(m_ViewportSize.x), uint32_t(m_ViewportSize.y));

		}

		Renderer2D::ResetStats();

		if (m_ViewportFocused && m_ViewportHovered)
		{
			m_CameraController.OnUpdate(ts);
			m_EditorCamera.OnUpdate(ts);
		}

		// Update Scene



		// Render
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		RenderCommand::Clear();

		// Clear our entity ID Attachment to -1
		
		m_Framebuffer->ClearAttachment(1, -1);

		//SO_CLIENT_INFO("Delta Time : {0}s  ({1}ms)", ts.GetSeconds(), 1000 / ts.GetMilliSeconds());


		//Renderer2D::BeginScene(m_CameraController.GetCamera());

		m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

		//Renderer2D::EndScene();

		auto [mx, my] = ImGui::GetMousePos();

		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;

		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

		my = m_ViewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;
		SO_CLIENT_WARN("Mouse Pos  : {0} {1}", mouseX, mouseY);
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)m_ViewportSize.x && mouseY < (int)m_ViewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		}

		m_Framebuffer->Unbind();


		/*
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->UploadUniformFloat4("u_Color", m_SquareColor);
		*/
	}

	void EditorLayer::OnImGuiRender()
	{
		SO_PROFILE_FUNCTION();

		auto stats = Renderer2D::GetStats();


		static bool dockspaceOpen = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minimumWindowSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 350.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minimumWindowSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Crtl+N"))
				{
					NewScene();
				}
				if (ImGui::MenuItem("Open File ...", "Ctrl+O"))
				{
					OpenScene();
				}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}


				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				if (ImGui::MenuItem("Exit")) Application::GetApplication().Close();


				ImGui::EndMenu();
			}


			ImGui::EndMenuBar();
		}

		m_SceneHierarchyPanel.OnImGuiRender();

		ImGui::Begin("Settings");
		std::string name = "None";
		if (m_HoveredEntity)
		{
			name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		}
		ImGui::Text("Hovered Entity: %s",name.c_str());

		ImGui::Text("Renderer2D Stats");
		ImGui::Text("Draw Calls : %d", stats.DrawCalls);
		ImGui::Text("Quads : %d", stats.QuadCount);
		ImGui::Text("Vertices : %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices : %d", stats.GetTotalIndexCount());

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();

		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::GetApplication().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		//SO_CLIENT_WARN("viewport size : {0}  {1}", viewportPanelSize.x, viewportPanelSize.y);

		uint32_t framebufferTexture = m_Framebuffer->GetColorAttachmentRendererID();

		ImGui::Image((void*)(uint64_t)m_Framebuffer->GetColorAttachmentRendererID(), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });




		//SO_CLIENT_WARN("MIN BOUNDS : {0} {1}", m_ViewportBounds[0].x, m_ViewportBounds[0].y);
		//SO_CLIENT_WARN("MAX BOUNDS : {0} {1}", m_ViewportBounds[1].x, m_ViewportBounds[1].y);

		// Gizmos

		bool snap = Input::IsKeyPressed(SO_KEY_LEFT_CONTROL);

		float snapValue = 0.50f; // Snap value for translation/scale

		if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
		{
			snapValue = 45.0f; // snap value for rotation
		}

		float snapValues[3] = { snapValue, snapValue, snapValue };

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			
			
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Can be used for runtime camera in the future
			//	auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			//	const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			//	const glm::mat4& cameraProjection = camera.GetProjectionMatrix();
			//	glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());


			// Editor Camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity Transform

			auto& tc = selectedEntity.GetComponent<TransformComponent>();

			glm::mat4 transform = tc.GetTransform();

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}


		ImGui::End();

		ImGui::PopStyleVar();

		ImGui::End();



		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_CameraController.OnEvent(e);
		m_EditorCamera.OnEvent(e);
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(SO_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(SO_BIND_EVENT_FN(EditorLayer::OnMousePressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(SO_KEY_LEFT_CONTROL) || Input::IsKeyPressed(SO_KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(SO_KEY_LEFT_SHIFT) || Input::IsKeyPressed(SO_KEY_RIGHT_SHIFT);
		switch (e.GetKeyCode())
		{
		case SO_KEY_O:
			if (control)
			{
				OpenScene();
			}
			break;
		case SO_KEY_N:
			if (control)
			{
				NewScene();
			}
			break;
		case SO_KEY_S:
			if (control && shift)
			{
				SaveSceneAs();
			}
			break;

			// Gizmo
		case SO_KEY_A:
			m_GizmoType = -1;
			break;

		case SO_KEY_Z:
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;


		case SO_KEY_E:
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;

		case SO_KEY_R:
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;

		}
		return false;
	}

	bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == SO_MOUSE_BUTTON_LEFT)
		{
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(SO_KEY_LEFT_ALT))
			{
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
			}
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = std::make_shared<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filePath = FileDialogs::OpenFile("SpaceOptima Planogram (*.so)\0*.so\0");
		if (!filePath.empty())
		{
			SceneSerializer sceneSerializer(m_ActiveScene);
			sceneSerializer.Serialize(filePath);
		}
	}

	void EditorLayer::OpenScene()
	{
		std::string filePath = FileDialogs::OpenFile("SpaceOptima Planogram (*.so)\0*.so\0");
		if (!filePath.empty())
		{
			m_ActiveScene = std::make_shared<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
			SceneSerializer sceneSerializer(m_ActiveScene);
			sceneSerializer.Deserialize(filePath);
		}
	}

}