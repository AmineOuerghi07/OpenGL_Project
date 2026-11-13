#include "fepch.h"
#include "RendererSelect.h"
#include "RendererAPI.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>

#if defined(FE_PLATFORM_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
#elif defined(FE_PLATFORM_MACOS) || defined(FE_PLATFORM_LINUX)
	#include <unistd.h>
	#include <spawn.h>
	#include <limits.h>
	extern char** environ;
	#if defined(FE_PLATFORM_MACOS)
		#include <mach-o/dyld.h>
	#endif
#endif

namespace FunEngine
{
	namespace
	{
		static std::vector<std::string> s_Args;

		static std::string ToLower(std::string s)
		{
			std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
			return s;
		}

		static bool TryParseRendererToken(const std::string& token, RendererAPI::API& out)
		{
			const std::string t = ToLower(token);
			if (t == "opengl" || t == "gl")
			{
				out = RendererAPI::API::OpenGL;
				return true;
			}
			if (t == "vulkan" || t == "vk")
			{
				out = RendererAPI::API::Vulkan;
				return true;
			}
			return false;
		}

		static bool ParseCli(int argc, char** argv, RendererAPI::API& out, bool& found)
		{
			for (int i = 1; i < argc; ++i)
			{
				std::string arg = argv[i];
				if (arg.rfind("--renderer=", 0) == 0)
				{
					if (TryParseRendererToken(arg.substr(11), out))
					{
						found = true;
						return true;
					}
					FE_CORE_ERROR("Unknown renderer in --renderer=: {0}", arg.substr(11));
					return false;
				}
				if (arg == "--renderer" && i + 1 < argc)
				{
					if (TryParseRendererToken(argv[i + 1], out))
					{
						found = true;
						return true;
					}
					FE_CORE_ERROR("Unknown renderer: {0}", argv[i + 1]);
					return false;
				}
			}
			return true;
		}

		static bool ParseEnv(RendererAPI::API& out, bool& found)
		{
			const char* env = std::getenv("FE_RENDERER");
			if (!env || !env[0])
				return true;

			if (TryParseRendererToken(env, out))
			{
				found = true;
				return true;
			}
			FE_CORE_ERROR("Unknown FE_RENDERER value: {0}", env);
			return false;
		}

		static bool ParseConfigFile(RendererAPI::API& out, bool& found)
		{
			std::ifstream file("funengine.cfg");
			if (!file)
				return true;

			std::string line;
			while (std::getline(file, line))
			{
				const auto pos = line.find('=');
				if (pos == std::string::npos)
					continue;

				std::string key = ToLower(line.substr(0, pos));
				std::string value = line.substr(pos + 1);
				value.erase(0, value.find_first_not_of(" \t"));
				value.erase(value.find_last_not_of(" \t\r\n") + 1);

				if (key == "renderer" && TryParseRendererToken(value, out))
				{
					found = true;
					return true;
				}
				if (key == "renderer")
				{
					FE_CORE_ERROR("Unknown renderer in funengine.cfg: {0}", value);
					return false;
				}
			}
			return true;
		}

		static std::vector<std::string> BuildRelaunchArgs(RendererAPI::API api)
		{
			std::vector<std::string> out;
			if (!s_Args.empty())
				out.push_back(s_Args[0]);
			else
				out.emplace_back();

			for (size_t i = 1; i < s_Args.size(); ++i)
			{
				const std::string& a = s_Args[i];
				if (a.rfind("--renderer=", 0) == 0)
					continue;
				if (a == "--renderer")
				{
					++i; // skip value
					continue;
				}
				out.push_back(a);
			}
			out.push_back(std::string("--renderer=") + RendererSelect_APIName(api));
			return out;
		}

		static bool SpawnRelaunch(const std::vector<std::string>& args)
		{
			if (args.empty() || args[0].empty())
			{
				FE_CORE_ERROR("Cannot relaunch: executable path unknown");
				return false;
			}

#if defined(FE_PLATFORM_WINDOWS)
			std::ostringstream cmd;
			for (size_t i = 0; i < args.size(); ++i)
			{
				if (i) cmd << ' ';
				const bool needsQuotes = args[i].find(' ') != std::string::npos;
				if (needsQuotes) cmd << '"';
				cmd << args[i];
				if (needsQuotes) cmd << '"';
			}
			STARTUPINFOA si{};
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi{};
			std::string mutableCmd = cmd.str();
			if (!CreateProcessA(nullptr, mutableCmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
			{
				FE_CORE_ERROR("CreateProcess failed ({0})", (int)GetLastError());
				return false;
			}
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			FE_CORE_INFO("Relaunched: {0}", mutableCmd);
			return true;
#elif defined(FE_PLATFORM_MACOS) || defined(FE_PLATFORM_LINUX)
			std::vector<char*> argv;
			argv.reserve(args.size() + 1);
			for (const auto& a : args)
				argv.push_back(const_cast<char*>(a.c_str()));
			argv.push_back(nullptr);

			pid_t pid = 0;
			const int rc = posix_spawn(&pid, args[0].c_str(), nullptr, nullptr, argv.data(), environ);
			if (rc != 0)
			{
				FE_CORE_ERROR("posix_spawn failed ({0})", rc);
				return false;
			}
			FE_CORE_INFO("Relaunched pid {0}: {1} {2}", (int)pid, args[0], args.size() > 1 ? args[1] : "");
			return true;
#else
			(void)args;
			return false;
#endif
		}

		static std::string ResolveExecutablePath()
		{
#if defined(FE_PLATFORM_WINDOWS)
			char buf[MAX_PATH]{};
			const DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
			if (n == 0 || n >= MAX_PATH)
				return s_Args.empty() ? std::string() : s_Args[0];
			return std::string(buf, buf + n);
#elif defined(FE_PLATFORM_MACOS)
			char buf[PATH_MAX]{};
			uint32_t size = sizeof(buf);
			if (_NSGetExecutablePath(buf, &size) == 0)
				return std::string(buf);
			return s_Args.empty() ? std::string() : s_Args[0];
#else
			return s_Args.empty() ? std::string() : s_Args[0];
#endif
		}
	}

	void RendererSelect_RememberArgs(int argc, char** argv)
	{
		s_Args.clear();
		s_Args.reserve((size_t)std::max(argc, 1));
		for (int i = 0; i < argc; ++i)
			s_Args.emplace_back(argv[i] ? argv[i] : "");

		const std::string exe = ResolveExecutablePath();
		if (!exe.empty())
		{
			if (s_Args.empty())
				s_Args.push_back(exe);
			else
				s_Args[0] = exe;
		}
	}

	bool RendererSelect_IsVulkanCompiledIn()
	{
#ifdef FE_WITH_VULKAN
		return true;
#else
		return false;
#endif
	}

	const char* RendererSelect_APIName(RendererAPI::API api)
	{
		switch (api)
		{
		case RendererAPI::API::Vulkan: return "vulkan";
		case RendererAPI::API::OpenGL: return "opengl";
		default: return "opengl";
		}
	}

	bool RendererSelect_WriteConfig(RendererAPI::API api)
	{
		std::ifstream in("funengine.cfg");
		std::vector<std::string> lines;
		bool replaced = false;
		if (in)
		{
			std::string line;
			while (std::getline(in, line))
			{
				std::string key = line;
				const auto pos = key.find('=');
				if (pos != std::string::npos)
				{
					key = ToLower(key.substr(0, pos));
					key.erase(key.find_last_not_of(" \t") + 1);
					if (key == "renderer")
					{
						lines.push_back(std::string("renderer=") + RendererSelect_APIName(api));
						replaced = true;
						continue;
					}
				}
				lines.push_back(line);
			}
		}
		if (!replaced)
			lines.push_back(std::string("renderer=") + RendererSelect_APIName(api));

		std::ofstream out("funengine.cfg", std::ios::trunc);
		if (!out)
		{
			FE_CORE_ERROR("Failed to write funengine.cfg");
			return false;
		}
		for (const auto& l : lines)
			out << l << '\n';
		FE_CORE_INFO("Wrote funengine.cfg renderer={0}", RendererSelect_APIName(api));
		return true;
	}

	bool RendererSelect_RelaunchWith(RendererAPI::API api)
	{
		if (api == RendererAPI::API::Vulkan && !RendererSelect_IsVulkanCompiledIn())
		{
			FE_CORE_ERROR("Cannot relaunch with Vulkan: this build lacks FE_WITH_VULKAN");
			return false;
		}

		if (!RendererSelect_WriteConfig(api))
			return false;

		auto args = BuildRelaunchArgs(api);
		if (args.empty() || args[0].empty())
			args = { ResolveExecutablePath(), std::string("--renderer=") + RendererSelect_APIName(api) };

		return SpawnRelaunch(args);
	}

	bool RendererSelect_HandleSetRendererAndExit(int argc, char** argv)
	{
		for (int i = 1; i < argc; ++i)
		{
			std::string arg = argv[i] ? argv[i] : "";
			std::string token;
			if (arg.rfind("--set-renderer=", 0) == 0)
				token = arg.substr(15);
			else if (arg == "--set-renderer" && i + 1 < argc)
				token = argv[++i] ? argv[i] : "";
			else
				continue;

			RendererAPI::API api = RendererAPI::API::OpenGL;
			if (!TryParseRendererToken(token, api))
			{
				FE_CORE_ERROR("Unknown --set-renderer value: {0}", token);
				std::exit(1);
			}
			if (api == RendererAPI::API::Vulkan && !RendererSelect_IsVulkanCompiledIn())
			{
				FE_CORE_ERROR("--set-renderer=vulkan requires a --with-vulkan build");
				std::exit(1);
			}
			if (!RendererSelect_WriteConfig(api))
				std::exit(1);
			return true;
		}
		return false;
	}

	bool RendererSelect_Apply(int argc, char** argv)
	{
		RendererSelect_RememberArgs(argc, argv);

		RendererAPI::API selected = RendererAPI::API::OpenGL;
		bool found = false;

		if (!ParseConfigFile(selected, found))
			return false;

		if (!ParseEnv(selected, found))
			return false;

		if (!ParseCli(argc, argv, selected, found))
			return false;

		if (selected == RendererAPI::API::Vulkan)
		{
#ifndef FE_WITH_VULKAN
			FE_CORE_ERROR("Vulkan renderer requested but this build was compiled without FE_WITH_VULKAN (rebuild with --with-vulkan)");
			return false;
#else
			RendererAPI::SetAPI(RendererAPI::API::Vulkan);
			FE_CORE_INFO("Renderer: Vulkan");
			return true;
#endif
		}

		RendererAPI::SetAPI(RendererAPI::API::OpenGL);
		FE_CORE_INFO("Renderer: OpenGL");
		return true;
	}
}
