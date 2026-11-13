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
		bool IsInitialized() const { return m_Initialized; }
		// True between Begin() and End() when ImGui widgets may be submitted this frame.
		bool IsFrameBegun() const { return m_FrameBegun; }
	private:
		bool m_BlockEvents;
		float m_Time;
		bool m_Initialized = false;
		bool m_FrameBegun = false;
	};
}