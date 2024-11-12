#pragma once

#include "glm/glm.hpp"

namespace SpaceOptima
{
	class Camera
	{
	public:

		Camera() = default;

		Camera(const glm::mat4& projection);

		virtual ~Camera() = default;

		glm::mat4 GetProjectionMatrix() const { return m_Projection; }

	protected:
		glm::mat4 m_Projection;
	};
}