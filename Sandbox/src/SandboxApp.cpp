#include <FunEngine.h>
#include <FunEngine/Core/EntryPoint.h>


#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLShader.h"
#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include "Sandbox2D.h"

class ExampleLayer : public FunEngine::Layer
{
public:
	ExampleLayer() :Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
	{
		//Vertex Array
		m_VertexArray = FunEngine::VertexArray::Create();
		//Vertex Buffer

		float vertices[3 * 7] =
		{
			-0.5f, -0.5f, 0.0f, 0.6f, 0.2f, 0.2f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.6f, 0.2f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.2f, 0.2f, 0.6f, 1.0f
		};

		FunEngine::Ref<FunEngine::VertexBuffer> vertexBuffer;

		vertexBuffer = FunEngine::VertexBuffer::Create(vertices, sizeof(vertices));
		//m_VertexBuffer->Bind();
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		FunEngine::BufferLayout layout = {
			{FunEngine::ShaderDataType::Float3, "a_Position"},
			{FunEngine::ShaderDataType::Float4, "a_Color"}

		};

		vertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(vertexBuffer);
		/*glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);*/

		unsigned int indices[3] = { 0, 1, 2 };
		FunEngine::Ref<FunEngine::IndexBuffer> indexBuffer;
		indexBuffer = FunEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));

		m_VertexArray->SetIndexBuffer(indexBuffer);
		//Index Buffer


		float squareVertices[5 * 4] =
		{
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		m_SquareVA = (FunEngine::VertexArray::Create());
		FunEngine::Ref<FunEngine::VertexBuffer> SquareVB;
		SquareVB = FunEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		FunEngine::BufferLayout squareVBLayout = {
			{FunEngine::ShaderDataType::Float3, "a_Position"},
			{FunEngine::ShaderDataType::Float2, "a_TextCoord"}
		};

		SquareVB->SetLayout(squareVBLayout);

		m_SquareVA->AddVertexBuffer(SquareVB);

		unsigned int squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		FunEngine::Ref < FunEngine::IndexBuffer > SquareIB;
		SquareIB = (FunEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		m_SquareVA->SetIndexBuffer(SquareIB);

		std::string vertexSrc = R"(
		#version 330 core

		layout(location = 0) in vec3 a_Position;
		layout(location = 1) in vec4 a_Color;
		out vec3 v_Position;
		out vec4 v_Color;
		
		uniform mat4 u_ViewProjection;
		uniform mat4 u_Transform;


		void main()
		{
			v_Position = a_Position;
			v_Color = a_Color;
			gl_Position = u_ViewProjection * u_Transform * vec4(a_Position,1.0);
		}

		)";

		std::string fragmentSrc = R"(
		#version 330 core
		
		layout(location = 0) out vec4 color;
		in vec3 v_Position;
		in vec4 v_Color;
		void main()
		{
			color = vec4(v_Position * 0.5 + 0.5, 1.0);
			color = v_Color;
		}

		)";


		m_Shader = FunEngine::Shader::Create("FirstShader",vertexSrc, fragmentSrc);


		std::string blueShaderVertexSrc = R"(
		#version 330 core

		layout(location = 0) in vec3 a_Position;
		layout(location = 1) in vec4 a_Color;
		out vec3 v_Position;
		out vec4 v_Color;

		uniform mat4 u_ViewProjection;
		uniform mat4 u_Transform;

		void main()
		{
			v_Position = a_Position;
			v_Color = a_Color;
			gl_Position = u_ViewProjection * u_Transform * vec4(a_Position,1.0);
		}

		)";

		std::string blueShaderFragmentSrc = R"(
		#version 330 core
		
		layout(location = 0) out vec4 color;
		in vec3 v_Position;
		in vec4 v_Color;

		uniform vec3 u_Color;

		void main()
		{
			color = vec4(u_Color, 1.0);
		}

		)";


		m_BlueShader = FunEngine::Shader::Create("BlueShader",blueShaderVertexSrc, blueShaderFragmentSrc);





		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = FunEngine::Texture2D::Create("assets/textures/textureTest.jpg");
		
		m_TextureRGBA = FunEngine::Texture2D::Create("assets/textures/textureRGBA.png");

		m_SquareColor = { 0.9f, 0.2f, 0.2f };


		std::dynamic_pointer_cast<FunEngine::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<FunEngine::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);

	}
	void OnUpdate(FunEngine::Timestep ts) override 
	{
		
		//FE_CLIENT_TRACE("Delta Time : {0}s  ({1}ms)", ts.GetSeconds(), ts.GetMilliSeconds());




		/*if (FunEngine::Input::IsKeyPressed(FE_KEY_J))
		{
			m_SquarePosition.x -= m_SquareMoveSpeed * ts;
		}
		else if (FunEngine::Input::IsKeyPressed(FE_KEY_L))
		{
			m_SquarePosition.x += m_SquareMoveSpeed * ts;
		}
		if (FunEngine::Input::IsKeyPressed(FE_KEY_K))
		{
			m_SquarePosition.y -= m_SquareMoveSpeed * ts;
		}
		else if (FunEngine::Input::IsKeyPressed(FE_KEY_I))
		{
			m_SquarePosition.y += m_SquareMoveSpeed * ts;
		}*/

		//glClearColor(0.1f,0.1f,0.1f,0);
			//glClear(GL_COLOR_BUFFER_BIT);

		// Update
		m_CameraController.OnUpdate(ts);

		// Render
		FunEngine::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		FunEngine::RenderCommand::Clear();

	

		FunEngine::Renderer::BeginScene(m_CameraController.GetCamera());

		//m_BlueShader->Bind();
		//m_BlueShader->UploadUniformMat4("u_ViewProjection", m_Camera.GetViewProjectionMatrix());

		

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		

		//std::dynamic_pointer_cast<FunEngine::OpenGLShader>(m_Shader)->Bind();

		std::dynamic_pointer_cast<FunEngine::OpenGLShader>(m_BlueShader)->Bind();
		std::dynamic_pointer_cast<FunEngine::OpenGLShader>(m_BlueShader)->UploadUniformFloat3("u_Color", m_SquareColor);
		
		
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				
				FunEngine::Renderer::Submit(m_BlueShader, m_SquareVA, transform);

			}
		}
		
		m_Texture->Bind();
		
		auto textureShader = m_ShaderLibrary.Get("Texture");

		FunEngine::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		m_TextureRGBA->Bind();

		FunEngine::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));


		// Triangle 
		//FunEngine::Renderer::Submit(m_Shader, m_VertexArray);
		
		
		//m_SquareVA->Bind();
		//m_Shader->Bind();
		//m_BlueShader->UploadUniformMat4("u_ViewProjection", m_Camera.GetViewProjectionMatrix());
	

		FunEngine::Renderer::EndScene();
		//glDrawElements(GL_TRIANGLES, m_SquareVA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		//m_Shader->Bind();
		//m_VertexArray->Bind();
		//glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

	}
	void OnEvent(FunEngine::Event& e) override
	{
		/*FunEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<FunEngine::KeyPressedEvent>(FE_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));*/
		m_CameraController.OnEvent(e);

	}

	//bool OnKeyPressedEvent(FunEngine::KeyPressedEvent& event)
	//{
	//	if (event.GetKeyCode() == FE_KEY_LEFT)
	//	{
	//		m_CameraPosition.x -= m_CameraSpeed;
	//	}

	//	if (event.GetKeyCode() == FE_KEY_RIGHT)
	//	{
	//		m_CameraPosition.x += m_CameraSpeed;
	//	}

	//	if (event.GetKeyCode() == FE_KEY_UP)
	//	{
	//		m_CameraPosition.y += m_CameraSpeed;
	//	}

	//	if (event.GetKeyCode() == FE_KEY_DOWN)
	//	{
	//		m_CameraPosition.y -= m_CameraSpeed;
	//	}

	//	return false;
	//}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("SquareColor", glm::value_ptr(m_SquareColor));

		ImGui::End();

	}
	private:

		FunEngine::ShaderLibrary m_ShaderLibrary;

		FunEngine::Ref<FunEngine::Shader> m_Shader;
		FunEngine::Ref<FunEngine::Shader> m_BlueShader;

		FunEngine::Ref<FunEngine::VertexArray> m_VertexArray;
		FunEngine::Ref<FunEngine::VertexArray> m_SquareVA;

		FunEngine::Ref<FunEngine::Texture2D> m_Texture;
		FunEngine::Ref<FunEngine::Texture2D> m_TextureRGBA;

		FunEngine::OrthographicCameraController m_CameraController;
		

		glm::vec3 m_SquareColor;

};
class Sandbox : public FunEngine::Application
{
	public:
		Sandbox()
		{
			//
			// PushLayer(new ExampleLayer());
			PushLayer(new Sandbox2D());
		
		};
		~Sandbox()
		{

		};
};

FunEngine::Application* FunEngine::CreateApplication()
{
	return new Sandbox();
}