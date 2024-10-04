#include "sopch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace SpaceOptima
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}