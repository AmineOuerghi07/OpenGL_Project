#include "Sandbox2D.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLShader.h"
#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>



Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, false)
{
}

void Sandbox2D::OnAttach()
{

	m_Texture = SpaceOptima::Texture2D::Create("assets/textures/textureTest.jpg");


}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(SpaceOptima::Timestep ts)
{
	SO_PROFILE_FUNCTION();
	// Update
	// Reset here
	SpaceOptima::Renderer2D::ResetStats();
	{
		SO_PROFILE_SCOPE("CameraOnUpdate");
		m_CameraController.OnUpdate(ts);
	}
	// Render
	{

		SO_PROFILE_SCOPE("RenderCommand SetColor");
		SpaceOptima::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
		SpaceOptima::RenderCommand::Clear();


	}
	
	{
		
		{
			
			static float rotation = 0.0f;
			rotation += ts * 30.0f;
			//SO_CORE_INFO("rotation = {0}", rotation);
			//SO_CLIENT_TRACE("Delta Time : {0}s  ({1}ms)", ts.GetSeconds(), ts.GetMilliSeconds());
			SO_PROFILE_SCOPE("Renderer RenderScene");
			SpaceOptima::Renderer2D::BeginScene(m_CameraController.GetCamera());

			SpaceOptima::Renderer2D::DrawRotatedQuad({ -1.0f, 1.0f }, { 0.8f, 0.8f }, { 0.2f, 0.2f, 0.8f, 1.0f }, rotation);
			SpaceOptima::Renderer2D::DrawRotatedQuad({ 1.0f, -1.0f }, { 0.8f, 0.8f }, { 0.2f, 0.2f, 0.8f, 1.0f }, rotation);
			SpaceOptima::Renderer2D::DrawQuad({ 0.5f, 0.5f }, { 0.5f, 0.75f }, { 0.8f, 0.2f, 0.2f, 1.0f });
			SpaceOptima::Renderer2D::DrawQuad({ -0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.8f, 0.2f, 1.0f });

			SpaceOptima::Renderer2D::DrawQuad({ 0.0f, 0.0f , -0.1f }, { 10.0f, 10.0f }, m_Texture, 1.0f);
			SpaceOptima::Renderer2D::DrawRotatedQuad({ 0.0f, 0.0f , 0.0f }, { 1.0f, 1.0f }, m_Texture, rotation, 1.0f, { 0.5f, 0.2f, 0.2f, 1.0f });


			SpaceOptima::Renderer2D::EndScene();


			SpaceOptima::Renderer2D::BeginScene(m_CameraController.GetCamera());

			

			for (float y = -10.0f; y < 10.0f; y +=0.5f)
			{
				for (float x = -10.f; x < 10.f; x += 0.5f)
				{
					glm::vec4 color = {(x + 10.0f) / 10.0f, 0.4f, (y + 10.0f) / 10.0f, 1.0f};
					SpaceOptima::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
				}
			}
			SpaceOptima::Renderer2D::EndScene();
			
		}
	}

	/*
	std::dynamic_pointer_cast<SpaceOptima::OpenGLShader>(m_Shader)->Bind();
	std::dynamic_pointer_cast<SpaceOptima::OpenGLShader>(m_Shader)->UploadUniformFloat4("u_Color", m_SquareColor);
	*/
}

void Sandbox2D::OnImGuiRender()
{
	SO_PROFILE_FUNCTION();

	auto stats = SpaceOptima::Renderer2D::GetStats();
	ImGui::Begin("Settings");
	ImGui::Text("Renderer2D Stats");
	ImGui::Text("Draw Calls : %d", stats.DrawCalls);
	ImGui::Text("Quads : %d", stats.QuadCount);
	ImGui::Text("Vertices : %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices : %d", stats.GetTotalIndexCount());


	ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_SquareColor));

	ImGui::End();
}

void Sandbox2D::OnEvent(SpaceOptima::Event& e)
{
	m_CameraController.OnEvent(e);
}
