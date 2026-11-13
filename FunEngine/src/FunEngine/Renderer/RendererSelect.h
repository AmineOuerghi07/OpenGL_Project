#pragma once

#include "FunEngine/Renderer/RendererAPI.h"
#include <string>
#include <vector>

namespace FunEngine
{
	/// Parses CLI, FE_RENDERER, and optional funengine.cfg; sets RendererAPI. Default OpenGL.
	bool RendererSelect_Apply(int argc, char** argv);

	/// If argv contains `--set-renderer=opengl|vulkan`, write funengine.cfg and return true (caller should exit 0).
	bool RendererSelect_HandleSetRendererAndExit(int argc, char** argv);

	/// Keep argv so Settings can relaunch with a replaced --renderer flag.
	void RendererSelect_RememberArgs(int argc, char** argv);

	/// True when this binary was compiled with Premake --with-vulkan (FE_WITH_VULKAN).
	bool RendererSelect_IsVulkanCompiledIn();

	/// Write `renderer=opengl|vulkan` to funengine.cfg in the current working directory.
	bool RendererSelect_WriteConfig(RendererAPI::API api);

	/// Write cfg, spawn a new process with the same args (forcing --renderer=...), then return true
	/// if the child was started (caller should Close the app). Returns false if relaunch failed.
	bool RendererSelect_RelaunchWith(RendererAPI::API api);

	const char* RendererSelect_APIName(RendererAPI::API api);
}
