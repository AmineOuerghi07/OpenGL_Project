#pragma once
#include "SpaceOptima/Window.h"

#include <GLFW/glfw3.h>

namespace SpaceOptima
{
	
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& windowProps);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetHeight() const override { return m_Data.Height; }
		inline unsigned int GetWidth() const override { return m_Data.Width; }

		// Window Attribute

		inline void SetEventCallBack(const EventCallBackFn& callback) override { m_Data.EventCallBack = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual inline void* GetNativeWindow() const override { return m_Window; }

	private:
		virtual void Init(const WindowProps& windowProps);
		virtual void Shutdown();
	

	private:
		GLFWwindow* m_Window;
		struct WindowData
		{
			std::string Title;
			unsigned int Height;
			unsigned int Width;
			bool VSync;

			EventCallBackFn EventCallBack;
			
		};

		WindowData m_Data;
	};

}