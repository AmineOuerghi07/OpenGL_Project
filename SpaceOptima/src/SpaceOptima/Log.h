#pragma once
#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace SpaceOptima {
	class SO_API Log
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
#define SO_CORE_ERROR(...)	::SpaceOptima::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SO_CORE_WARN(...)	::SpaceOptima::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SO_CORE_TRACE(...)	::SpaceOptima::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SO_CORE_INFO(...)	::SpaceOptima::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SO_CORE_FATAL(...)	::SpaceOptima::Log::GetCoreLogger()->fatal(__VA_ARGS__)
//client log macros
#define SO_CLIENT_ERROR(...)	::SpaceOptima::Log::GetClientLogger()->error(__VA_ARGS__)
#define SO_CLIENT_WARN(...)	::SpaceOptima::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SO_CLIENT_TRACE(...)	::SpaceOptima::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SO_CLIENT_INFO(...)	::SpaceOptima::Log::GetClientLogger()->info(__VA_ARGS__)
#define SO_CLIENT_FATAL(...)	::SpaceOptima::Log::GetClientLogger()->fatal(__VA_ARGS__)