#pragma once
#include "FunEngine/Core/Core.h"
#include "FunEngine/Debug/Instrumentor.h"

#ifdef FE_PLATFORM_WINDOWS

extern FunEngine::Application* FunEngine::CreateApplication();

int main(int argc, char** argv)
{
	FunEngine::Log::Init();
	FE_PROFILE_BEGIN_SESSION("Startup", "FunEngine-StartUp.json");
	auto app = FunEngine::CreateApplication();
	FE_PROFILE_END_SESSION();
	FE_PROFILE_BEGIN_SESSION("Startup", "FunEngine-Running.json");
	app->Run();
	FE_PROFILE_END_SESSION();
	FE_PROFILE_BEGIN_SESSION("Startup", "FunEngine-Shutdown.json");
	delete app;
	FE_PROFILE_END_SESSION();
}

#endif