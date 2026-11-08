VULKAN_SDK = os.getenv("VULKAN_SDK")


workspace "FunEngine"
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
IncludeDir["GLFW"] ="FunEngine/vendor/GLFW/include"
IncludeDir["Glad"] ="FunEngine/vendor/Glad/include"
IncludeDir["imgui"] ="FunEngine/vendor/imgui"
IncludeDir["glm"] ="FunEngine/vendor/glm"
IncludeDir["stb_image"] ="FunEngine/vendor/stb_image"
IncludeDir["entt"] ="FunEngine/vendor/entt/include"
IncludeDir["yaml_cpp"] ="FunEngine/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] ="FunEngine/vendor/ImGuizmo"
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
	include "FunEngine/vendor/GLFW"
	include "FunEngine/vendor/Glad"
	include "FunEngine/vendor/imgui"
	include "FunEngine/vendor/yaml-cpp"
	group ""

project "FunEngine"

	location "FunEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	pchheader "fepch.h"
	pchsource "FunEngine/src/fepch.cpp"
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
	
	filter "files:FunEngine/vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "files:FunEngine/vendor/glm/glm/**.cppm"
	flags { "NoPCH" }

	filter "files:FunEngine/vendor/glm/glm/detail/**.cpp"
	flags { "NoPCH" }

	filter "files:FunEngine/vendor/shaderc/libshaderc/include/**.hpp"
	flags { "NoPCH" }
	
	filter "files:FunEngine/vendor/shaderc/libshaderc/include/**.h"
	flags { "NoPCH" }

	filter "files:FunEngine/vendor/spirv/spirv_cross/**.cpp"
	flags { "NoPCH" }

	filter "files:FunEngine/vendor/spirv/spirv_cross/**.hpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"
		defines 
		{
			"FE_PLATFORM_WINDOWS",
			"FE_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}


	filter "configurations:Debug"
		defines "FE_ENGINE_DEBUG"
		runtime "Debug"
		symbols "on"
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

		
	filter "configurations:Release"
		defines "FE_ENGINE_RELEASE"
		runtime "Release"
		optimize "on"
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}



	filter "configurations:Dist"
		defines "FE_ENGINE_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}
	


project "FunEngine_Editor"
	location "FunEngine_Editor"
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
		"FunEngine/vendor/spdlog/include",
		"FunEngine/src/",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"FunEngine/vendor/imgui/",
		"%{IncludeDir.ImGuizmo}"
	}

	links 
	{
		"FunEngine"
	}

	filter "system:windows"
		systemversion "latest"
		defines 
		{
			"FE_PLATFORM_WINDOWS",
		}
		
	filter "configurations:Debug"
		defines "FE_ENGINE_DEBUG"
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		defines "FE_ENGINE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "FE_ENGINE_DIST"
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
			"FunEngine/vendor/spdlog/include",
			"FunEngine/src/",
			"%{IncludeDir.glm}",
			"FunEngine/vendor/imgui/",
			"%{IncludeDir.entt}"
		}
	
		links 
		{
			"FunEngine"
		}
	
		filter "system:windows"
			systemversion "latest"
			defines 
			{
				"FE_PLATFORM_WINDOWS",
			}
			
		filter "configurations:Debug"
			defines "FE_ENGINE_DEBUG"
			runtime "Debug"
			symbols "on"
		filter "configurations:Release"
			defines "FE_ENGINE_RELEASE"
			runtime "Release"
			optimize "on"
	
		filter "configurations:Dist"
			defines "FE_ENGINE_DIST"
			runtime "Release"
			optimize "on"