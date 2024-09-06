#pragma once
#include "SpaceOptima/Core.h"
#include "SpaceOptima/Events/Event.h"
namespace SpaceOptima{
	class SO_API  Layer
{
public:
	Layer(const std::string& name = "layer");
	virtual ~Layer();
	virtual void  OnAttach() {}
	virtual void  OnDetach() {}
	virtual void  OnUpdate() {}
	virtual void  OnEvent(Event& e) {}

	inline const std::string GetName() const { return m_DebugName; }

protected:
	std::string m_DebugName;
};
}
