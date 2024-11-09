#include "sopch.h"
#include "WindowsWindow.h"
#include "SpaceOptima/Events/ApplicationEvent.h"
#include "SpaceOptima/Events/KeyEvent.h"
#include "SpaceOptima/Events/MouseEvent.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <glad/glad.h>
namespace SpaceOptima
{
	static bool s_GLFWInitialized = false;
	static void GLFWErrorCallback(int error, const char* description) 
	{
		SO_CORE_ERROR("GLFW ERROR ({0}): {1}", error, description);
	}

	Scope<Window> Window::Create(const WindowProps& windowProps)
	{
		return std::make_unique<WindowsWindow>(windowProps);
	}

	WindowsWindow::WindowsWindow(const WindowProps& windowProps)
	{
		Init(windowProps);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& windowProps)
	{
		m_Data.Title = windowProps.Title;
		m_Data.Height = windowProps.Height;
		m_Data.Width = windowProps.Width;

		SO_CORE_INFO("Creating Window {0} (Width: {1} Height {2}) ", m_Data.Title, m_Data.Width, m_Data.Height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			SO_CORE_ASSERT(success, "GLFW is not initialized !!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		
		m_context = new OpenGLContext(m_Window);
		m_context->Init();
		
		
		glfwSetWindowUserPointer(m_Window, &m_Data);

		SetVSync(false);
		

		//

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window,int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallBack(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallBack(event);

		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) 
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallBack(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallBack(event);
					break;
				}
				case GLFW_RELEASE:
				{ 
					KeyReleasedEvent event(key);
					data.EventCallBack(event);
					break;
				}
			}
		
		});
		glfwSetCharCallback(m_Window, [](GLFWwindow* window,unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent event(keycode);
				data.EventCallBack(event);

			});
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) 
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallBack(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallBack(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrolledEvent event((float) xOffset, (float) yOffset);
			data.EventCallBack(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallBack(event);
		});
		
	}



	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
		m_Data.VSync = enabled;
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		m_context->SwapBuffers();
		
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
}