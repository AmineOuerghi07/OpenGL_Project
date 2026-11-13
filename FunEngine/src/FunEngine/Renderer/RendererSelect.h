#pragma once

namespace FunEngine
{
	/// Parses CLI, FE_RENDERER, and optional funengine.cfg; sets RendererAPI. Default OpenGL.
	bool RendererSelect_Apply(int argc, char** argv);
}
