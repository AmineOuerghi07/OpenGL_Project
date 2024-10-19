#pragma once

#include <SpaceOptima.h>

class Sandbox2D : public SpaceOptima::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;
	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(SpaceOptima::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(SpaceOptima::Event& e) override;

private:

	SpaceOptima::OrthographicCameraController m_CameraController;


	// Temporary
	SpaceOptima::Ref<SpaceOptima::Texture2D> m_Texture;
	SpaceOptima::Ref<SpaceOptima::VertexArray> m_VertexArray;
	SpaceOptima::Ref<SpaceOptima::Shader> m_Shader;
	SpaceOptima::Ref<SpaceOptima::VertexArray> m_SquareVA;



	glm::vec4 m_SquareColor = { 0.8f, 0.0f, 0.1f, 1.0f};

};