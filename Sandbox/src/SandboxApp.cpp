#include <SpaceOptima.h>
class ExampleLayer : public SpaceOptima::Layer
{
public:
	ExampleLayer() :Layer("Example")
	{}
	void OnUpdate() override {
		SO_CLIENT_INFO("ExampleLayer::Update");
	}
	void OnEvent(SpaceOptima::Event& event) override
	{
		SO_CLIENT_TRACE("{0}",event);
	}
};
class Sandbox : public SpaceOptima::Application
{
	public:
		Sandbox()
		{
			PushLayer(new ExampleLayer());
			PushOverlay(new SpaceOptima::ImGuiLayer());
		};
		~Sandbox()
		{

		};
};

SpaceOptima::Application* SpaceOptima::CreateApplication()
{
	return new Sandbox();
}