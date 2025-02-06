VULKAN_SDK = os.getenv("VULKAN_SDK")


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
IncludeDir["glm"] ="SpaceOptima/vendor/glm"
IncludeDir["stb_image"] ="SpaceOptima/vendor/stb_image"
IncludeDir["entt"] ="SpaceOptima/vendor/entt/include"
IncludeDir["yaml_cpp"] ="SpaceOptima/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] ="SpaceOptima/vendor/ImGuizmo"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

group "Dependencies"
	include "SpaceOptima/vendor/GLFW"
	include "SpaceOptima/vendor/Glad"
	include "SpaceOptima/vendor/imgui"
	include "SpaceOptima/vendor/yaml-cpp"
	group ""

project "SpaceOptima"

	location "SpaceOptima"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	pchheader "sopch.h"
	pchsource "SpaceOptima/src/sopch.cpp"
	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",

		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/glm/glm/**.h",
		"%{prj.name}/vendor/glm/glm/**.cppm",
		"%{prj.name}/vendor/glm/glm/**.cpp",

		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.VulkanSDK}"

	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"YAML_CPP_STATIC_DEFINE"
	}
	
	links 
	{
		"GLFW",
		"Glad",
		"imgui",
		"opengl32.lib",
		"yaml-cpp"
	}
	
	filter "files:SpaceOptima/vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "files:SpaceOptima/vendor/glm/glm/**.cppm"
	flags { "NoPCH" }

	filter "files:SpaceOptima/vendor/glm/glm/detail/**.cpp"
	flags { "NoPCH" }

	filter "files:SpaceOptima/vendor/shaderc/libshaderc/include/**.hpp"
	flags { "NoPCH" }
	
	filter "files:SpaceOptima/vendor/shaderc/libshaderc/include/**.h"
	flags { "NoPCH" }

	filter "files:SpaceOptima/vendor/spirv/spirv_cross/**.cpp"
	flags { "NoPCH" }

	filter "files:SpaceOptima/vendor/spirv/spirv_cross/**.hpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		defines 
		{
			"SO_PLATFORM_WINDOWS",
			"SO_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}


	filter "configurations:Debug"
		defines "SO_ENGINE_DEBUG"
		runtime "Debug"
		symbols "on"
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

		
	filter "configurations:Release"
		defines "SO_ENGINE_RELEASE"
		runtime "Release"
		optimize "on"
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}



	filter "configurations:Dist"
		defines "SO_ENGINE_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}
	


project "SpaceOptima_Editor"
	location "SpaceOptima_Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
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
		"SpaceOptima/src/",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"SpaceOptima/vendor/imgui/",
		"%{IncludeDir.ImGuizmo}"
	}

	links 
	{
		"SpaceOptima"
	}

	filter "system:windows"
		systemversion "latest"
		defines 
		{
			"SO_PLATFORM_WINDOWS",
		}
		
	filter "configurations:Debug"
		defines "SO_ENGINE_DEBUG"
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		defines "SO_ENGINE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SO_ENGINE_DIST"
		runtime "Release"
		optimize "on"



		project "Sandbox"
		location "Sandbox"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++20"
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
			"SpaceOptima/src/",
			"%{IncludeDir.glm}",
			"SpaceOptima/vendor/imgui/",
			"%{IncludeDir.entt}"
		}
	
		links 
		{
			"SpaceOptima"
		}
	
		filter "system:windows"
			systemversion "latest"
			defines 
			{
				"SO_PLATFORM_WINDOWS",
			}
			
		filter "configurations:Debug"
			defines "SO_ENGINE_DEBUG"
			runtime "Debug"
			symbols "on"
		filter "configurations:Release"
			defines "SO_ENGINE_RELEASE"
			runtime "Release"
			optimize "on"
	
		filter "configurations:Dist"
			defines "SO_ENGINE_DIST"
			runtime "Release"
			optimize "on"