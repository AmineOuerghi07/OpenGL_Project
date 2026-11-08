#pragma once

#include "FunEngine/Core/Core.h"

namespace FunEngine
{
	class FE_API Input
	{
	public:
		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}