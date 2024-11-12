#include "sopch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"

#include "SpaceOptima/Renderer/Renderer2D.h"

#include "SpaceOptima/Renderer/Camera.h"

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

		Camera* primaryCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;

		auto view = m_Registry.view<CameraComponent, TransformComponent>();
		for (auto entity : view)
		{
			const auto& [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);

			if (camera.Primary)
			{
				primaryCamera = &camera.Camera;
				cameraTransform = &transform.Transform;
				break;
			}
		}


		if (primaryCamera)
		{
			Renderer2D::BeginScene(*primaryCamera, *cameraTransform);
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				const auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.Color);
			}

			Renderer2D::EndScene();
		}
	}
	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-fixed aspect ratio cameras
		
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	
	}

}