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
		virtual void OnImGuiRender() override;
		void Begin();
		void End();

	private:
		float m_Time;
	};
}