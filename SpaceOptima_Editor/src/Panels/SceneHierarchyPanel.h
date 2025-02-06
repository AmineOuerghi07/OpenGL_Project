#pragma once

#include <SpaceOptima/Core/Core.h>
#include <SpaceOptima/Scene/Scene.h>
#include <SpaceOptima/Scene/Entity.h>
#include <SpaceOptima/Renderer/Texture.h>

namespace SpaceOptima
{
	class SceneHierarchyPanel
	{
	public:

		SceneHierarchyPanel();

		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_SelectionContext; }

		void SetSelectedEntity(Entity entity);

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		Ref<Scene> m_Context;

		Entity m_SelectionContext;

		Ref<Texture2D> m_TextureHolder;

	};
}