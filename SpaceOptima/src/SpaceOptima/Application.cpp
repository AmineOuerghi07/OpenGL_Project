#include "sopch.h"
#include "Application.h"
#include "SpaceOptima/Log.h"
#include "SpaceOptima/input.h"
#include "SpaceOptima/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace SpaceOptima
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	

	Application::Application()
	{
		SO_CORE_ASSERT(!s_Instance, "Application already exists !!!");

		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_ImGuiLayer = new ImGuiLayer();
		m_Window->SetEventCallBack(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);
		PushOverlay(m_ImGuiLayer);

	
	}

	Application::~Application()
	{

	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		//SO_CORE_INFO("{0}", e);
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled())
			{
				break;
			}
		}
	}

	void Application::Run()
	{
		
		while (m_Running)
		{
			float time = (float) glfwGetTime(); // Should be in platform Platform::GetTime()
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
			
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate(timestep);
			}

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
			auto [x, y] = Input::GetMousePosition();
			//SO_CORE_TRACE("{0}, {1}", x, y);
			m_Window->OnUpdate();
			
		}
	}
	void Application::PushLayer(Layer * layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
}