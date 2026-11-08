#pragma once

#include <FunEngine/Core/Core.h>
#include <FunEngine/Scene/Scene.h>
#include <FunEngine/Scene/Entity.h>
#include <FunEngine/Renderer/Texture.h>

namespace FunEngine
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