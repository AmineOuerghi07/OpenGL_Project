#pragma once

#include <memory>

// Must precede any spdlog/fmt include (also from TU that skip fepch).
#if defined(FE_PLATFORM_MACOS)
	#ifndef FMT_CONSTEVAL
		#define FMT_CONSTEVAL
	#endif
#endif

#if defined(FE_PLATFORM_WINDOWS)
	#if FE_DYNAMIC_LINK
		#ifdef FE_BUILD_DLL
			#define FE_API _declspec(dllexport)
		#else
			#define FE_API _declspec(dllimport)
		#endif
	#else
		#define FE_API
	#endif
#elif defined(FE_PLATFORM_MACOS) || defined(FE_PLATFORM_LINUX)
	#define FE_API
#else
	#error FunEngine: unknown platform — define FE_PLATFORM_WINDOWS, FE_PLATFORM_MACOS, or FE_PLATFORM_LINUX
#endif

#ifdef FE_ENGINE_DEBUG
	#define FE_ENABLE_ASSERTS
#endif

#if defined(FE_PLATFORM_WINDOWS)
	#define FE_DEBUGBREAK() __debugbreak()
#elif defined(FE_PLATFORM_MACOS) || defined(FE_PLATFORM_LINUX)
	#define FE_DEBUGBREAK() __builtin_debugtrap()
#else
	#define FE_DEBUGBREAK()
#endif

#ifdef FE_ENABLE_ASSERTS
	#define FE_CLIENT_ASSERT(x, ...) { if(!(x)) { FE_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); FE_DEBUGBREAK(); }}
	#define FE_CORE_ASSERT(x, ...) { if(!(x)) { FE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); FE_DEBUGBREAK(); }}
#else
	#define FE_CLIENT_ASSERT(x, ...)
	#define FE_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
#define FE_BIND_EVENT_FN(fn) std::bind (&fn, this, std::placeholders::_1)

namespace FunEngine
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr <T>;
}
