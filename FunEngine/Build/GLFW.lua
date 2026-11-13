-- First-party Premake overlay for vendored GLFW (Cocoa/NSGL on macOS).
-- Lives outside the GLFW submodule so we do not commit into third-party gitlinks.
project "GLFW"
	kind "StaticLib"
	language "C"
	staticruntime "on"
	location "../vendor/GLFW"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"../vendor/GLFW/include/GLFW/glfw3.h",
		"../vendor/GLFW/include/GLFW/glfw3native.h",
		"../vendor/GLFW/src/glfw_config.h",
		"../vendor/GLFW/src/mappings.h",
		"../vendor/GLFW/src/internal.h",
		"../vendor/GLFW/src/platform.h",
		"../vendor/GLFW/src/null_platform.h",
		"../vendor/GLFW/src/null_joystick.h",
		"../vendor/GLFW/src/null_init.c",
		"../vendor/GLFW/src/null_monitor.c",
		"../vendor/GLFW/src/null_window.c",
		"../vendor/GLFW/src/null_joystick.c",
		"../vendor/GLFW/src/platform.c",
		"../vendor/GLFW/src/context.c",
		"../vendor/GLFW/src/init.c",
		"../vendor/GLFW/src/input.c",
		"../vendor/GLFW/src/monitor.c",
		"../vendor/GLFW/src/vulkan.c",
		"../vendor/GLFW/src/window.c"
	}

	filter "system:windows"
		systemversion "latest"

		files
		{
			"../vendor/GLFW/src/win32_platform.h",
			"../vendor/GLFW/src/win32_joystick.h",
			"../vendor/GLFW/src/win32_time.h",
			"../vendor/GLFW/src/win32_thread.h",
			"../vendor/GLFW/src/win32_init.c",
			"../vendor/GLFW/src/win32_joystick.c",
			"../vendor/GLFW/src/win32_monitor.c",
			"../vendor/GLFW/src/win32_time.c",
			"../vendor/GLFW/src/win32_thread.c",
			"../vendor/GLFW/src/win32_window.c",
			"../vendor/GLFW/src/win32_module.c",
			"../vendor/GLFW/src/wgl_context.c",
			"../vendor/GLFW/src/egl_context.c",
			"../vendor/GLFW/src/osmesa_context.c"
		}

		defines
		{
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "system:macosx"
		files
		{
			"../vendor/GLFW/src/cocoa_platform.h",
			"../vendor/GLFW/src/cocoa_joystick.h",
			"../vendor/GLFW/src/cocoa_init.m",
			"../vendor/GLFW/src/cocoa_joystick.m",
			"../vendor/GLFW/src/cocoa_monitor.m",
			"../vendor/GLFW/src/cocoa_window.m",
			"../vendor/GLFW/src/cocoa_time.h",
			"../vendor/GLFW/src/cocoa_time.c",
			"../vendor/GLFW/src/nsgl_context.m",
			"../vendor/GLFW/src/posix_thread.h",
			"../vendor/GLFW/src/posix_thread.c",
			"../vendor/GLFW/src/posix_module.c",
			"../vendor/GLFW/src/posix_poll.h",
			"../vendor/GLFW/src/posix_poll.c",
			"../vendor/GLFW/src/egl_context.c",
			"../vendor/GLFW/src/osmesa_context.c"
		}

		defines
		{
			"_GLFW_COCOA"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
