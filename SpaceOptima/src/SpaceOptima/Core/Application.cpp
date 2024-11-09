#include "sopch.h"
#include "Application.h"
#include "SpaceOptima/Core/Log.h"
#include "SpaceOptima/Core/Input.h"
#include "SpaceOptima/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

// PR Testing Ysf
namespace SpaceOptima
{

	#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	

	Application::Application(const std::string& name)
	{
		SO_CORE_ASSERT(!s_Instance, "Application already exists !!!");

		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(name)));
		m_ImGuiLayer = new ImGuiLayer();
		m_Window->SetEventCallBack(BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

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
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
		//SO_CORE_INFO("{0}", e);
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
			if (e.Handled)
			{
				break;
			}
			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		
		while (m_Running)
		{
			float time = (float) glfwGetTime(); // Should be in platform Platform::GetTime()
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(timestep);
				}
			}
			
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
			
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

	void Application::Close()
	{
		m_Running = false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;

		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}