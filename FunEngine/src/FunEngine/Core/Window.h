#pragma once
#include "FunEngine/Core/Core.h"
#include "FunEngine/Events/Event.h"
namespace FunEngine
{
	class GraphicsContext;

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Fun Engine",
			unsigned int width = 1600,
			unsigned int height = 900)
			: Title(title), Width(width), Height(height)
		{

		}
	};


	class FE_API Window
	{
	public:
		using EventCallBackFn = std::function<void(Event&)>;

		virtual ~Window() {};

		virtual void OnUpdate() = 0;
		
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetWidth() const = 0;

		// Window Attributes 

		virtual void SetEventCallBack(const EventCallBackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual void* GetNativeWindow() const = 0;
		virtual GraphicsContext* GetGraphicsContext() const { return nullptr; }

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}