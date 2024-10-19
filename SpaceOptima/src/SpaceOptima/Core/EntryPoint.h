#pragma once
#include "SpaceOptima/Core/Core.h"
#include "SpaceOptima/Debug/Instrumentor.h"

#ifdef SO_PLATFORM_WINDOWS

extern SpaceOptima::Application* SpaceOptima::CreateApplication();

int main(int argc, char** argv)
{
	SpaceOptima::Log::Init();
	SO_PROFILE_BEGIN_SESSION("Startup", "SpaceOptima-StartUp.json");
	auto app = SpaceOptima::CreateApplication();
	SO_PROFILE_END_SESSION();
	SO_PROFILE_BEGIN_SESSION("Startup", "SpaceOptima-Running.json");
	app->Run();
	SO_PROFILE_END_SESSION();
	SO_PROFILE_BEGIN_SESSION("Startup", "SpaceOptima-Shutdown.json");
	delete app;
	SO_PROFILE_END_SESSION();
}

#endif