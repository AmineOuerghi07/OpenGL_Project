#pragma once
#include "SpaceOptima/Layer.h"
#include "SpaceOptima/Events/ApplicationEvent.h"
#include "SpaceOptima/Events/KeyEvent.h"
#include "SpaceOptima/Events/MouseEvent.h"
namespace SpaceOptima
{
	class SO_API ImGuiLayer : public Layer
	{
	public:
		 ImGuiLayer();
		~ImGuiLayer();
		void  OnAttach() override;
		void  OnDetach() override;
		void  OnUpdate() override;
		void  OnEvent(Event& e) override;
	private:
		bool OnMouseMovedEvent(MouseMovedEvent& e);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
		bool OnMouseScrolledEvent(MouseScrolledEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnKeyReleasedEvent(KeyReleasedEvent& e);
		bool OnKeyTypedEvent(KeyTypedEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);
	private:
		float m_Time;
	};
}