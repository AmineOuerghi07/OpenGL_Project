#pragma once

#include <FunEngine.h>

class Sandbox2D : public FunEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;
	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(FunEngine::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(FunEngine::Event& e) override;

private:

	FunEngine::OrthographicCameraController m_CameraController;


	// Temporary
	FunEngine::Ref<FunEngine::Texture2D> m_Texture;
	FunEngine::Ref<FunEngine::Texture2D> m_TextureMilk;
	FunEngine::Ref<FunEngine::VertexArray> m_VertexArray;
	FunEngine::Ref<FunEngine::Shader> m_Shader;
	FunEngine::Ref<FunEngine::VertexArray> m_SquareVA;
	FunEngine::Ref<FunEngine::Framebuffer> m_Framebuffer;
	bool m_RenderToFramebuffer = true;



	glm::vec4 m_SquareColor = { 0.8f, 0.0f, 0.1f, 1.0f};

};