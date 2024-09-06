workspace "SpaceOptima"
	startproject "Sandbox"
	architecture "x64"
	configurations 
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] ="SpaceOptima/vendor/GLFW/include"
IncludeDir["Glad"] ="SpaceOptima/vendor/Glad/include"
IncludeDir["imgui"] ="SpaceOptima/vendor/imgui"
include "SpaceOptima/vendor/GLFW"
include "SpaceOptima/vendor/Glad"
include "SpaceOptima/vendor/imgui"

project "SpaceOptima"

	location "SpaceOptima"
	kind "SharedLib"
	language "C++"
	staticruntime "off"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	pchheader "sopch.h"
	pchsource "SpaceOptima/src/sopch.cpp"
	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}"

	}
	links 
	{
		"GLFW",
		"Glad",
		"imgui",
		"opengl32.lib"
	}
	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"
		defines 
		{
			"SO_PLATFORM_WINDOWS",
			"SO_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}
		
		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
		}
	filter "configurations:Debug"
		defines "SO_ENGINE_DEBUG"
		runtime "Debug"
		symbols "On"
	filter "configurations:Release"
		defines "SO_ENGINE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "SO_ENGINE_DIST"
		runtime "Release"
		optimize "On"
	


project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs 
	{
		"SpaceOptima/vendor/spdlog/include",
		"SpaceOptima/src/"

	}

	links 
	{
		"SpaceOptima"
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"
		defines 
		{
			"SO_PLATFORM_WINDOWS",
		}
		
	filter "configurations:Debug"
		defines "SO_ENGINE_DEBUG"
		runtime "Debug"
		symbols "On"
	filter "configurations:Release"
		defines "SO_ENGINE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "SO_ENGINE_DIST"
		runtime "Release"
		optimize "On"