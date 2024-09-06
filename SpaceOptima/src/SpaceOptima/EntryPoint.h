#pragma once

#ifdef SO_PLATFORM_WINDOWS

extern SpaceOptima::Application* SpaceOptima::CreateApplication();

int main(int argc, char** argv)
{
	SpaceOptima::Log::Init();
	SO_CORE_WARN("Initialized Log");
	int testVariable = 4;
	SO_CLIENT_INFO("Initialized Log Var={0}",testVariable);
	auto app = SpaceOptima::CreateApplication();
	app->Run();
	delete app;
}

#endif