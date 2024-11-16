#include "sopch.h"

#include "Entity.h"

namespace SpaceOptima
{
	Entity::Entity(entt::entity handle, Scene* scene)
		:m_EntityHandle(handle), m_Scene(scene)
	{
	}
	Entity::~Entity()
	{
	}
}