#pragma once
#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace FunEngine {
	class FE_API Log
	{		
		public:
		
			static void Init();
			inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger;}
			inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger;}
		private:
		    static	std::shared_ptr<spdlog::logger> s_CoreLogger;
		    static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}
//core log macros
#define FE_CORE_ERROR(...)	::FunEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define FE_CORE_WARN(...)	::FunEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define FE_CORE_TRACE(...)	::FunEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define FE_CORE_INFO(...)	::FunEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define FE_CORE_FATAL(...)	::FunEngine::Log::GetCoreLogger()->fatal(__VA_ARGS__)
//client log macros
#define FE_CLIENT_ERROR(...)	::FunEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define FE_CLIENT_WARN(...)	::FunEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define FE_CLIENT_TRACE(...)	::FunEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define FE_CLIENT_INFO(...)	::FunEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define FE_CLIENT_FATAL(...)	::FunEngine::Log::GetClientLogger()->fatal(__VA_ARGS__)