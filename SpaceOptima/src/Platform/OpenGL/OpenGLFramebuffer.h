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

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { return m_ColorAttachments[index]; }

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_DepthAttachment = 0;
		FramebufferSpecification m_Specifications;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentsSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpecs = FramebufferTextureFormat::None;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachments = 0;
	};

}