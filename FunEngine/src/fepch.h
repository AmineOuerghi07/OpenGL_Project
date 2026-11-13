#pragma once
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>

// Apple Clang + vendored spdlog/fmt: fmt's consteval format-string checks
// fail against current Apple libc++. Force FMT_CONSTEVAL empty before spdlog.
#if defined(FE_PLATFORM_MACOS)
	#ifndef FMT_CONSTEVAL
		#define FMT_CONSTEVAL
	#endif
#endif

#include "FunEngine/Core/Log.h"

#include "FunEngine/Debug/Instrumentor.h"

#ifdef FE_PLATFORM_WINDOWS
#include <Windows.h>
#endif
