#include "fepch.h"
#include "Camera.h"

namespace FunEngine
{
	Camera::Camera(const glm::mat4& projection)
		:m_Projection(projection)
	{
	}
}