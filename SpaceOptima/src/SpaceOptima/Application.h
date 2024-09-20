#pragma once
#include "Core.h"
#include "SpaceOptima/LayerStack.h"
#include "Events/Event.h"
#include "SpaceOptima/Window.h"
#include "SpaceOptima/Events/ApplicationEvent.h"
#include "SpaceOptima/ImGui/ImGuiLayer.h"
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
			std::unique_ptr<Window> m_Window;
			ImGuiLayer* m_ImGuiLayer;
			bool m_Running = true;
			LayerStack m_LayerStack;

			static Application* s_Instance;
	};

	Application* CreateApplication();
}

