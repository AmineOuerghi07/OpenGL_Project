#include "sopch.h"
#include "OpenGLContext.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
namespace SpaceOptima
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_windowHandle(windowHandle)
	{
		SO_CORE_ASSERT(m_windowHandle, "Window handle is null");
	}
	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		SO_CORE_ASSERT(status, "Failed to initialize Glad");
		

		SO_CORE_INFO("OpenGL Info:");
		SO_CORE_INFO("  Vendor: {0}", (char*)glGetString(GL_VENDOR));
		SO_CORE_INFO("  Renderer: {0}", (char*)glGetString(GL_RENDERER));
		SO_CORE_INFO("  Version: {0}",(char*) glGetString(GL_VERSION));
	}
	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_windowHandle);
	}
}