#pragma once

#include "SpaceOptima/Renderer/Framebuffer.h"

namespace SpaceOptima
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& specs);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specifications; }

		virtual uint32_t GetColorAttachementRendererID() const override { return m_ColorAttachement; }

		virtual void Resize(uint32_t width, uint32_t height) override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_ColorAttachement = 0, m_DepthAttachement = 0;
		FramebufferSpecification m_Specifications;
	};

}