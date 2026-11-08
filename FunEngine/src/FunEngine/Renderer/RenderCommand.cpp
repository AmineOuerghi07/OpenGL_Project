#include "fepch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace FunEngine
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}