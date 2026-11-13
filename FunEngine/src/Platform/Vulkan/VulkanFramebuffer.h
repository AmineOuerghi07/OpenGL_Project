#pragma once

#ifdef FE_WITH_VULKAN

#include "FunEngine/Renderer/Framebuffer.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"

namespace FunEngine
{
	// Offscreen target. Image orientation follows OpenGL texture memory (row 0 = bottom of the scene),
	// so ImGui::Image keeps the same flipped UVs and ReadPixel(x, y) keeps a bottom-left origin.
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		~VulkanFramebuffer() override;

		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

		uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { return 0; }
		void* GetColorAttachmentImGuiID(uint32_t index = 0) const override;

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;
		void ClearAttachment(uint32_t attachmentIndex, int value) override;

		// Used by VulkanContext while this framebuffer is the current render target.
		VulkanTargetInfo GetTargetInfo() const;
		void BeginPass(VkCommandBuffer cmd, bool clear, const float clearColor[4]);
		void EndPass(VkCommandBuffer cmd);
		void RecordClear(VkCommandBuffer cmd, const float clearColor[4]);
		bool IsPassActive() const { return m_PassActive; }

	private:
		struct ColorAttachment
		{
			FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
			VkFormat Format = VK_FORMAT_UNDEFINED;
			VulkanImageResource Image;
			mutable VkDescriptorSet ImGuiSet = VK_NULL_HANDLE;
			int ClearInt = -1;
		};

		void CreateRenderPasses();
		void Invalidate();
		void ReleaseTargets();
		void RecordPixelCopy(VkCommandBuffer cmd, uint32_t attachmentIndex, int x, int y, uint32_t slot);
		void RecordBeginPass(VkCommandBuffer cmd, VkRenderPass renderPass, const float clearColor[4]);

		FramebufferSpecification m_Specification;
		std::vector<ColorAttachment> m_Colors;
		FramebufferTextureSpecification m_DepthSpec;
		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
		VulkanImageResource m_Depth;

		VkRenderPass m_RenderPassClear = VK_NULL_HANDLE;
		VkRenderPass m_RenderPassLoad = VK_NULL_HANDLE;
		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;

		bool m_PassActive = false;

		VulkanBufferResource m_Readback;
	};
}

#endif
