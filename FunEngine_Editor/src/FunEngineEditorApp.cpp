#include <FunEngine.h>
#include <FunEngine/Core/EntryPoint.h>


#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLShader.h"
#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include "EditorLayer.h"

namespace FunEngine
{

	class FunEngineEditor : public Application
	{
	public:
		FunEngineEditor()
			:Application("FunEngine Editor")
		{
			//
			// PushLayer(new ExampleLayer());
			PushLayer(new EditorLayer());

		};
		~FunEngineEditor()
		{

		};
	};

	Application* CreateApplication()
	{
		return new FunEngineEditor();
	}

}