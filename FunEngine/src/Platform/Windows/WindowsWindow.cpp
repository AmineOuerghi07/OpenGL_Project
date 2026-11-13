#include "fepch.h"
#include "WindowsWindow.h"
#include "FunEngine/Events/ApplicationEvent.h"
#include "FunEngine/Events/KeyEvent.h"
#include "FunEngine/Events/MouseEvent.h"
#include "FunEngine/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLContext.h"

#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanContext.h"
#endif

#include <glad/glad.h>
namespace FunEngine
{
	static bool s_GLFWInitialized = false;
	static void GLFWErrorCallback(int error, const char* description) 
	{
		FE_CORE_ERROR("GLFW ERROR ({0}): {1}", error, description);
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

		FE_CORE_INFO("Creating Window {0} (Width: {1} Height {2}) ", m_Data.Title, m_Data.Width, m_Data.Height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			FE_CORE_ASSERT(success, "GLFW is not initialized !!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		const bool useVulkan = RendererAPI::GetAPI() == RendererAPI::API::Vulkan;

#ifdef FE_WITH_VULKAN
		if (useVulkan)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		else
#endif
		{
#ifdef FE_PLATFORM_MACOS
			// Apple OpenGL is frozen at ~4.1 core; request that profile before create.
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
		}

		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

#ifdef FE_WITH_VULKAN
		if (useVulkan)
			m_context = new VulkanContext(m_Window);
		else
#endif
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
		if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL)
		{
			if (enabled)
				glfwSwapInterval(1);
			else
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