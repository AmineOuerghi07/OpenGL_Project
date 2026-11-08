#pragma once
#include "Core.h"
#include "FunEngine/Core/LayerStack.h"
#include "FunEngine/Events/Event.h"
#include "FunEngine/Core/Window.h"
#include "FunEngine/Events/ApplicationEvent.h"
#include "FunEngine/ImGui/ImGuiLayer.h"
#include "FunEngine/Renderer/Shader.h"
#include "FunEngine/Renderer/Buffer.h"
#include "FunEngine/Renderer/VertexArray.h"

#include "FunEngine/Renderer/OrthographicCamera.h"

#include "FunEngine/Core/TimeStep.h"

namespace FunEngine
{
	class FE_API Application
	{
		public:
			Application(const std::string& name = "FunEngine Engine");
			virtual ~Application();
			void Run();
			void OnEvent(Event& e);
			void PushLayer(Layer* layer);
			void PushOverlay(Layer* overlay);

			inline static Application& GetApplication() { return *s_Instance; }
			inline Window& GetWindow() { return *m_Window; }

			void Close();

			ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		private:
			bool OnWindowClose(WindowCloseEvent& e);
			bool OnWindowResize(WindowResizeEvent& e);
		private:
			std::unique_ptr<Window> m_Window;
			ImGuiLayer* m_ImGuiLayer;
			bool m_Running = true;
			LayerStack m_LayerStack;
			float m_LastFrameTime = 0.0f;
			static Application* s_Instance;

			bool m_Minimized = false;
			
	};

	Application* CreateApplication();
}

