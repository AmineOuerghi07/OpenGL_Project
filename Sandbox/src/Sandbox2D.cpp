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




}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(SpaceOptima::Timestep ts)
{
	// Update
	m_CameraController.OnUpdate(ts);

	// Render

	SpaceOptima::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
	SpaceOptima::RenderCommand::Clear();



	SpaceOptima::Renderer2D::BeginScene(m_CameraController.GetCamera());

	SpaceOptima::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.2f, 0.2f, 0.8f, 1.0f });
	SpaceOptima::Renderer2D::DrawQuad({ 0.5f, 0.5f }, { 0.5f, 0.75f }, { 0.8f, 0.2f, 0.2f, 1.0f });


	SpaceOptima::Renderer2D::EndScene();


	/*
	std::dynamic_pointer_cast<SpaceOptima::OpenGLShader>(m_Shader)->Bind();
	std::dynamic_pointer_cast<SpaceOptima::OpenGLShader>(m_Shader)->UploadUniformFloat4("u_Color", m_SquareColor);
	*/
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_SquareColor));

	ImGui::End();
}

void Sandbox2D::OnEvent(SpaceOptima::Event& e)
{
	m_CameraController.OnEvent(e);
}
