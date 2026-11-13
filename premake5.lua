VULKAN_SDK = os.getenv("VULKAN_SDK") or ""

--------------------------------------------------------------------
-- Premake helpers (Premake 5.0 removed flags { "NoPCH" })
--------------------------------------------------------------------
-- Disable PCH for the current filter. Windows VS still needs this for
-- vendor files (ImGuizmo/glm/etc.). Premake 5 uses enablepch "Off"
-- (emits PrecompiledHeader=NotUsing); older Premake keeps flags{NoPCH}.
local function disable_pch()
	if enablepch ~= nil then
		enablepch "Off"
	elseif flags ~= nil then
		flags { "NoPCH" }
	end
end

newoption {
	trigger = "arch",
	value = "ARCHITECTURE",
	description = "Target CPU architecture (default: host). Use x64 or ARM64.",
	allowed = {
		{ "x64",   "x86_64" },
		{ "ARM64", "Apple Silicon / aarch64" },
	}
}

newoption {
	trigger = "with-vulkan",
	description = "Compile and link the Vulkan backend"
}

-- SPIR-V is prebuilt (scripts/compile_vulkan_shaders.sh). Do not link SDK shaderc /
-- SPIRV-Cross into FunEngine; those libs are optional tooling only.
if _OPTIONS["with-vulkan"] and (not VULKAN_SDK or VULKAN_SDK == "") then
	print("NOTE: --with-vulkan without VULKAN_SDK. macOS Homebrew is fine; on Windows set VULKAN_SDK to the LunarG SDK before building.")
end

local function resolve_architecture()
	if _OPTIONS["arch"] then
		return _OPTIONS["arch"]
	end
	-- Visual Studio projects must stay x64 when generated on an Apple Silicon Mac
	-- (otherwise the checked-in .sln/.vcxproj become ARM64 and break Windows).
	local action = _ACTION or ""
	if action:match("^vs") then
		return "x64"
	end
	-- Host-native for gmake/xcode on Apple Silicon.
	if os.hostarch ~= nil and os.hostarch() == "ARM64" then
		return "ARM64"
	end
	return "x64"
end

--------------------------------------------------------------------
-- Workspace
--------------------------------------------------------------------
workspace "FunEngine"
	startproject "Sandbox"
	architecture (resolve_architecture())
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "FunEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "FunEngine/vendor/Glad/include"
IncludeDir["imgui"] = "FunEngine/vendor/imgui"
IncludeDir["glm"] = "FunEngine/vendor/glm"
IncludeDir["stb_image"] = "FunEngine/vendor/stb_image"
IncludeDir["entt"] = "FunEngine/vendor/entt/include"
IncludeDir["yaml_cpp"] = "FunEngine/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "FunEngine/vendor/ImGuizmo"
-- Windows Vulkan SDK paths (empty when unset — do not expand to "/Include").
IncludeDir["VulkanSDK"] = (VULKAN_SDK ~= "" and (VULKAN_SDK .. "/Include") or "")
LibraryDir = {}
LibraryDir["VulkanSDK"] = (VULKAN_SDK ~= "" and (VULKAN_SDK .. "/Lib") or "")

group "Dependencies"
	include "FunEngine/Build/GLFW"
	include "FunEngine/vendor/Glad"
	include "FunEngine/vendor/imgui"
	include "FunEngine/vendor/yaml-cpp"
group ""

--------------------------------------------------------------------
-- Engine
--------------------------------------------------------------------
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
		"%{prj.name}/src/**.mm",

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
		"%{IncludeDir.ImGuizmo}"
	}

	filter { "options:with-vulkan", "system:windows" }
		if IncludeDir["VulkanSDK"] ~= "" then
			includedirs { IncludeDir["VulkanSDK"] }
		end
		if LibraryDir["VulkanSDK"] ~= "" then
			libdirs { LibraryDir["VulkanSDK"] }
		end
		links { "vulkan-1" }

	filter { "not options:with-vulkan" }
		removefiles
		{
			"%{prj.name}/src/Platform/Vulkan/**"
		}

	filter "options:with-vulkan"
		defines { "FE_WITH_VULKAN" }

	filter { "options:with-vulkan", "system:macosx" }
		includedirs
		{
			"/opt/homebrew/include",
			"/usr/local/include"
		}
		libdirs
		{
			"/opt/homebrew/lib",
			"/usr/local/lib"
		}
		links
		{
			"vulkan",
			"Metal.framework"
		}

	filter {}

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
		"yaml-cpp"
	}

	-- ObjC++ (.mm) cannot reuse a C++-only PCH on Clang.
	filter "files:FunEngine/src/**.mm"
		disable_pch()

	-- Vendor translation units must not consume the engine PCH (Windows + Mac).
	filter "files:FunEngine/vendor/ImGuizmo/**.cpp"
		disable_pch()

	filter "files:FunEngine/vendor/glm/glm/**.cppm"
		disable_pch()

	filter "files:FunEngine/vendor/glm/glm/detail/**.cpp"
		disable_pch()

	filter "files:FunEngine/vendor/shaderc/libshaderc/include/**.hpp"
		disable_pch()

	filter "files:FunEngine/vendor/shaderc/libshaderc/include/**.h"
		disable_pch()

	filter "files:FunEngine/vendor/spirv/spirv_cross/**.cpp"
		disable_pch()

	filter "files:FunEngine/vendor/spirv/spirv_cross/**.hpp"
		disable_pch()

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"FE_PLATFORM_WINDOWS",
			"FE_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}
		links
		{
			"opengl32.lib"
		}

	filter "system:macosx"
		defines
		{
			"FE_PLATFORM_MACOS",
			"GLFW_INCLUDE_NONE",
			"FMT_CONSTEVAL="
		}
		removefiles
		{
			"%{prj.name}/src/Platform/Windows/WindowsPlatformUtils.cpp"
		}
		links
		{
			"OpenGL.framework",
			"Cocoa.framework",
			"IOKit.framework",
			"CoreVideo.framework",
			"AppKit.framework",
			"QuartzCore.framework"
		}

	filter "system:windows"
		removefiles
		{
			"%{prj.name}/src/Platform/Mac/**"
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

--------------------------------------------------------------------
-- Editor
--------------------------------------------------------------------
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
		"FunEngine",
		"GLFW",
		"Glad",
		"imgui",
		"yaml-cpp"
	}

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"FE_PLATFORM_WINDOWS",
		}

	filter "system:macosx"
		defines
		{
			"FE_PLATFORM_MACOS",
			"FMT_CONSTEVAL="
		}
		links
		{
			"OpenGL.framework",
			"Cocoa.framework",
			"IOKit.framework",
			"CoreVideo.framework",
			"AppKit.framework",
			"QuartzCore.framework"
		}

	filter "options:with-vulkan"
		defines { "FE_WITH_VULKAN" }

	filter { "options:with-vulkan", "system:macosx" }
		libdirs
		{
			"/opt/homebrew/lib",
			"/usr/local/lib"
		}
		links
		{
			"vulkan",
			"Metal.framework"
		}

	filter { "options:with-vulkan", "system:windows" }
		if IncludeDir["VulkanSDK"] ~= "" then
			includedirs { IncludeDir["VulkanSDK"] }
		end
		if LibraryDir["VulkanSDK"] ~= "" then
			libdirs { LibraryDir["VulkanSDK"] }
		end
		links { "vulkan-1" }

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

--------------------------------------------------------------------
-- Sandbox (was incorrectly nested under Editor Dist filter)
--------------------------------------------------------------------
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
		"FunEngine",
		"GLFW",
		"Glad",
		"imgui",
		"yaml-cpp"
	}

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"FE_PLATFORM_WINDOWS",
		}

	filter "system:macosx"
		defines
		{
			"FE_PLATFORM_MACOS",
			"FMT_CONSTEVAL="
		}
		links
		{
			"OpenGL.framework",
			"Cocoa.framework",
			"IOKit.framework",
			"CoreVideo.framework",
			"AppKit.framework",
			"QuartzCore.framework"
		}

	filter "options:with-vulkan"
		defines { "FE_WITH_VULKAN" }

	filter { "options:with-vulkan", "system:macosx" }
		libdirs
		{
			"/opt/homebrew/lib",
			"/usr/local/lib"
		}
		links
		{
			"vulkan",
			"Metal.framework"
		}

	filter { "options:with-vulkan", "system:windows" }
		if IncludeDir["VulkanSDK"] ~= "" then
			includedirs { IncludeDir["VulkanSDK"] }
		end
		if LibraryDir["VulkanSDK"] ~= "" then
			libdirs { LibraryDir["VulkanSDK"] }
		end
		links { "vulkan-1" }

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
