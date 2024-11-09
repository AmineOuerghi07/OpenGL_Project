#include <SpaceOptima.h>
#include <SpaceOptima/Core/EntryPoint.h>


#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLShader.h"
#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include "EditorLayer.h"

namespace SpaceOptima
{

	class SpaceOptimaEditor : public Application
	{
	public:
		SpaceOptimaEditor()
			:Application("SpaceOptima Editor")
		{
			//
			// PushLayer(new ExampleLayer());
			PushLayer(new EditorLayer());

		};
		~SpaceOptimaEditor()
		{

		};
	};

	Application* CreateApplication()
	{
		return new SpaceOptimaEditor();
	}

}