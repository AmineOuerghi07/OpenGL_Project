#pragma once
#include "SpaceOptima/Core/Layer.h"
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
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;
		void Begin();
		void End();
		void BlockEvents(bool blockEvents) { m_BlockEvents = blockEvents; }
		void SetDarkThemeColor();
	private:
		bool m_BlockEvents;
		float m_Time;
	};
}