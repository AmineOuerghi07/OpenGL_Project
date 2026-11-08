#pragma once
#include "FunEngine/Core/Layer.h"
#include "FunEngine/Events/ApplicationEvent.h"
#include "FunEngine/Events/KeyEvent.h"
#include "FunEngine/Events/MouseEvent.h"
namespace FunEngine
{
	class FE_API ImGuiLayer : public Layer
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