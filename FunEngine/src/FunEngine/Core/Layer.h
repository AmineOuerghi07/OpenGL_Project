#pragma once
#include "FunEngine/Core/Core.h"
#include "FunEngine/Events/Event.h"
#include "FunEngine/Core/TimeStep.h"

namespace FunEngine{
	class FE_API  Layer
{
public:
	Layer(const std::string& name = "layer");
	virtual ~Layer();
	virtual void  OnAttach() {}
	virtual void  OnDetach() {}
	virtual void  OnUpdate(Timestep timestep) {}
	virtual void OnImGuiRender() {}
	virtual void  OnEvent(Event& e) {}

	inline const std::string GetName() const { return m_DebugName; }

protected:
	std::string m_DebugName;
};
}
