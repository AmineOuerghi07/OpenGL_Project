#include "sopch.h"
#include "Camera.h"

namespace SpaceOptima
{
	Camera::Camera(const glm::mat4& projection)
		:m_Projection(projection)
	{
	}
}