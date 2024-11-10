#pragma once

#include "entt.hpp"
#include "SpaceOptima/Core/TimeStep.h"

namespace SpaceOptima
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string name);

		// TEMP
		entt::registry& Reg() { return m_Registry; }


		void OnUpdate(Timestep ts);
	private:
		entt::registry m_Registry;

		friend class Entity;
	};
}