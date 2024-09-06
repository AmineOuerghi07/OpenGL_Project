#pragma once

#ifdef SO_PLATFORM_WINDOWS
	#ifdef SO_BUILD_DLL
		#define SO_API _declspec(dllexport)
	#else
		#define SO_API _declspec(dllimport)
	#endif
#else
	#error SO ONLY SUPPORTS WINDOWS !
#endif
#ifdef SO_ENGINE_DEBUG
	#define SO_ENABLE_ASSERTS
#endif // SO_ENGINE_DEBUG

#ifdef SO_ENABLE_ASSERTS
	#define SO_CLIENT_ASSERT(x, ...) { if(!(x)) { SO_ENGINE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
	#define SO_CORE_ASSERT(x, ...) { if(!(x)) { SO_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
#else
	#define SO_CLIENT_ASSERT(x, ...)
	#define SO_CORE_ASSERT(x, ...)
#endif


#define BIT(x) (1 << x)
#define SO_BIND_EVENT_FN(fn) std::bind (&fn, this, std::placeholders::_1)