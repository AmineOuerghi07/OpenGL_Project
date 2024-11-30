#pragma once


#include "SpaceOptima/Core/TimeStep.h"
#include "SpaceOptima/Renderer/EditorCamera.h"

#include "entt.hpp"

namespace SpaceOptima
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string name);

		void DestroyEntity(Entity entity);


		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		//void OnUpdateRuntime(Timestep ts); // TODO in the future

		void OnViewportResize(uint32_t width, uint32_t height);


		Entity GetPrimaryCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:

		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		friend class Entity;

		friend class SceneHierarchyPanel;

		friend class SceneSerializer;
	};
}