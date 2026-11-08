#pragma once

#include <memory>

#ifdef FE_PLATFORM_WINDOWS
	#if FE_DYNAMIC_LINK
		#ifdef FE_BUILD_DLL
			#define FE_API _declspec(dllexport)
		#else
			#define FE_API _declspec(dllimport)
		#endif
	#else
		#define FE_API

#endif
#else
	#error FunEngine only supports Windows!
#endif
#ifdef FE_ENGINE_DEBUG
	#define FE_ENABLE_ASSERTS
#endif // FE_ENGINE_DEBUG

#ifdef FE_ENABLE_ASSERTS
	#define FE_CLIENT_ASSERT(x, ...) { if(!(x)) { FE_ENGINE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
	#define FE_CORE_ASSERT(x, ...) { if(!(x)) { FE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
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