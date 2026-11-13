#pragma once
#include "FunEngine/Core/Core.h"
#include "FunEngine/Debug/Instrumentor.h"
#include "FunEngine/Renderer/RendererSelect.h"

#if defined(FE_PLATFORM_WINDOWS) || defined(FE_PLATFORM_MACOS) || defined(FE_PLATFORM_LINUX)

extern FunEngine::Application* FunEngine::CreateApplication();

int main(int argc, char** argv)
{
	FunEngine::Log::Init();
	if (FunEngine::RendererSelect_HandleSetRendererAndExit(argc, argv))
		return 0;
	if (!FunEngine::RendererSelect_Apply(argc, argv))
		return 1;
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
