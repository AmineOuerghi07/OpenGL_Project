#pragma once
#include "Core.h"
#include "SpaceOptima/Core/LayerStack.h"
#include "SpaceOptima/Events/Event.h"
#include "SpaceOptima/Core/Window.h"
#include "SpaceOptima/Events/ApplicationEvent.h"
#include "SpaceOptima/ImGui/ImGuiLayer.h"
#include "SpaceOptima/Renderer/Shader.h"
#include "SpaceOptima/Renderer/Buffer.h"
#include "SpaceOptima/Renderer/VertexArray.h"

#include "SpaceOptima/Renderer/OrthographicCamera.h"

#include "SpaceOptima/Core/TimeStep.h"

namespace SpaceOptima
{
	class SO_API Application
	{
		public:
			Application();
			virtual ~Application();
			void Run();
			void OnEvent(Event& e);
			void PushLayer(Layer* layer);
			void PushOverlay(Layer* overlay);

			inline static Application& GetApplication() { return *s_Instance; }
			inline Window& GetWindow() { return *m_Window; }
		private:
			bool OnWindowClose(WindowCloseEvent& e);
			bool OnWindowResize(WindowResizeEvent& e);
		private:
			std::unique_ptr<Window> m_Window;
			ImGuiLayer* m_ImGuiLayer;
			bool m_Running = true;
			LayerStack m_LayerStack;
			float m_LastFrameTime;
			static Application* s_Instance;

			bool m_Minimized = false;
			
	};

	Application* CreateApplication();
}

