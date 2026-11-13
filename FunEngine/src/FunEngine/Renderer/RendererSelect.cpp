#include "fepch.h"
#include "RendererSelect.h"
#include "RendererAPI.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>

namespace FunEngine
{
	namespace
	{
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
	}

	bool RendererSelect_Apply(int argc, char** argv)
	{
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
