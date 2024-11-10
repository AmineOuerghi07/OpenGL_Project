#include "sopch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"

#include "SpaceOptima/Renderer/Renderer2D.h"

namespace SpaceOptima
{

	Scene::Scene()
	{
		entt::entity entity = m_Registry.create();
	}
	
	
	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();

		tag.Tag = name;

		tag.Tag = name.empty() ? "Product" : name;

		return entity;
	}


	void Scene::OnUpdate(Timestep ts)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			const auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform, sprite.Color);
		}
	}
}