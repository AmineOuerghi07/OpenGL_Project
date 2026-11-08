#include "fepch.h"

#include "Entity.h"

namespace FunEngine
{
	Entity::Entity(entt::entity handle, Scene* scene)
		:m_EntityHandle(handle), m_Scene(scene)
	{
	}
	Entity::~Entity()
	{
	}
}