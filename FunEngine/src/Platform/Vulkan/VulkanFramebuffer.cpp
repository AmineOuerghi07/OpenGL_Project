#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanUtils.h"

#include "backends/imgui_impl_vulkan.h"

#include <cstring>

namespace FunEngine
{
	namespace
	{
		static const uint32_t s_MaxFramebufferSize = 8192;

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			return format == FramebufferTextureFormat::DEPTH24STENCIL8;
		}

		static VkFormat ToVkColorFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:       return VK_FORMAT_R8G8B8A8_UNORM;
			case FramebufferTextureFormat::RED_INTEGER: return VK_FORMAT_R32_SINT;
			default: break;
			}
			FE_CORE_ASSERT(false, "Invalid framebuffer format");
			return VK_FORMAT_UNDEFINED;
		}

		// Last slot is used for immediate (outside frame) readbacks.
		static constexpr uint32_t kImmediateReadbackSlot = VulkanContext::MaxFramesInFlight;
	}

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto& attachment : m_Specification.Attachments.Attachments)
		{
			if (IsDepthFormat(attachment.TextureFormat))
			{
				m_DepthSpec = attachment;
			}
			else
			{
				ColorAttachment color;
				color.TextureFormat = attachment.TextureFormat;
				color.Format = ToVkColorFormat(attachment.TextureFormat);
				m_Colors.push_back(color);
			}
		}
		if (m_DepthSpec.TextureFormat != FramebufferTextureFormat::None)
			m_DepthFormat = VulkanDevice::Get().GetDepthStencilFormat();

		if (m_Specification.Sample > 1)
			FE_CORE_WARN("Vulkan framebuffer: multisampling is not implemented, using 1 sample");

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VulkanUtils::CheckVk(vkCreateSampler(VulkanDevice::Get().GetDevice(), &samplerInfo, nullptr, &m_Sampler), "vkCreateSampler(fb)");

		m_Readback = VulkanBuffer::CreateHostBuffer(sizeof(int32_t) * (VulkanContext::MaxFramesInFlight + 1), VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		auto* readback = static_cast<int32_t*>(m_Readback.Mapped);
		for (uint32_t i = 0; i < VulkanContext::MaxFramesInFlight + 1; ++i)
			readback[i] = -1;

		CreateRenderPasses();
		Invalidate();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		if (VulkanDevice::Get().GetDevice() == VK_NULL_HANDLE)
			return;

		if (auto* ctx = VulkanContext::Get())
		{
			if (ctx->GetCurrentFramebuffer() == this)
				ctx->SetCurrentFramebuffer(nullptr);
		}

		VulkanPipelineCache::Get().InvalidateRenderPass(m_RenderPassClear);
		ReleaseTargets();

		VkRenderPass rpClear = m_RenderPassClear;
		VkRenderPass rpLoad = m_RenderPassLoad;
		VkSampler sampler = m_Sampler;
		VulkanBufferResource readback = m_Readback;
		auto destroy = [rpClear, rpLoad, sampler, readback]() mutable
		{
			VkDevice device = VulkanDevice::Get().GetDevice();
			if (device == VK_NULL_HANDLE)
				return;
			vkDestroyRenderPass(device, rpClear, nullptr);
			vkDestroyRenderPass(device, rpLoad, nullptr);
			vkDestroySampler(device, sampler, nullptr);
			VulkanBuffer::DestroyBuffer(readback);
		};
		if (auto* ctx = VulkanContext::Get())
			ctx->DeferDestroy(destroy);
		else
			destroy();
	}

	void VulkanFramebuffer::CreateRenderPasses()
	{
		const bool hasDepth = m_DepthFormat != VK_FORMAT_UNDEFINED;

		for (int variant = 0; variant < 2; ++variant)
		{
			const bool clear = variant == 0;

			std::vector<VkAttachmentDescription> attachments;
			std::vector<VkAttachmentReference> colorRefs;
			for (uint32_t i = 0; i < m_Colors.size(); ++i)
			{
				VkAttachmentDescription desc{};
				desc.format = m_Colors[i].Format;
				desc.samples = VK_SAMPLE_COUNT_1_BIT;
				desc.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				desc.initialLayout = clear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				attachments.push_back(desc);
				colorRefs.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			}

			VkAttachmentReference depthRef{ (uint32_t)attachments.size(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			if (hasDepth)
			{
				VkAttachmentDescription desc{};
				desc.format = m_DepthFormat;
				desc.samples = VK_SAMPLE_COUNT_1_BIT;
				desc.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				desc.stencilLoadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				desc.initialLayout = clear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				attachments.push_back(desc);
			}

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = (uint32_t)colorRefs.size();
			subpass.pColorAttachments = colorRefs.data();
			subpass.pDepthStencilAttachment = hasDepth ? &depthRef : nullptr;

			const VkPipelineStageFlags attachmentStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			const VkAccessFlags attachmentAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

			VkSubpassDependency dependencies[2]{};
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = attachmentStages | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | attachmentAccess;
			dependencies[0].dstStageMask = attachmentStages;
			dependencies[0].dstAccessMask = attachmentAccess;
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = attachmentStages;
			dependencies[1].srcAccessMask = attachmentAccess;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | attachmentStages;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | attachmentAccess;

			VkRenderPassCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = (uint32_t)attachments.size();
			info.pAttachments = attachments.data();
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 2;
			info.pDependencies = dependencies;

			VulkanUtils::CheckVk(vkCreateRenderPass(VulkanDevice::Get().GetDevice(), &info, nullptr,
				clear ? &m_RenderPassClear : &m_RenderPassLoad), "vkCreateRenderPass(fb)");
		}
	}

	void VulkanFramebuffer::ReleaseTargets()
	{
		VkFramebuffer framebuffer = m_Framebuffer;
		std::vector<VulkanImageResource> images;
		std::vector<VkDescriptorSet> imguiSets;
		for (auto& color : m_Colors)
		{
			images.push_back(color.Image);
			if (color.ImGuiSet != VK_NULL_HANDLE)
				imguiSets.push_back(color.ImGuiSet);
			color.Image = {};
			color.ImGuiSet = VK_NULL_HANDLE;
		}
		VulkanImageResource depth = m_Depth;

		m_Framebuffer = VK_NULL_HANDLE;
		m_Depth = {};

		auto destroy = [framebuffer, images, imguiSets, depth]() mutable
		{
			VkDevice device = VulkanDevice::Get().GetDevice();
			if (device == VK_NULL_HANDLE)
				return;
			if (VulkanContext::IsImGuiInitialized())
			{
				for (VkDescriptorSet set : imguiSets)
					ImGui_ImplVulkan_RemoveTexture(set);
			}
			if (framebuffer != VK_NULL_HANDLE)
				vkDestroyFramebuffer(device, framebuffer, nullptr);
			for (auto& image : images)
				VulkanBuffer::DestroyImage(image);
			VulkanBuffer::DestroyImage(depth);
		};
		if (auto* ctx = VulkanContext::Get())
			ctx->DeferDestroy(destroy);
		else
			destroy();
	}

	void VulkanFramebuffer::Invalidate()
	{
		if (m_Framebuffer != VK_NULL_HANDLE)
			ReleaseTargets();

		m_Specification.Width = std::clamp(m_Specification.Width, 1u, s_MaxFramebufferSize);
		m_Specification.Height = std::clamp(m_Specification.Height, 1u, s_MaxFramebufferSize);

		std::vector<VkImageView> views;
		for (auto& color : m_Colors)
		{
			color.Image = VulkanBuffer::CreateImage2D(m_Specification.Width, m_Specification.Height, color.Format,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			views.push_back(color.Image.View);
		}

		if (m_DepthFormat != VK_FORMAT_UNDEFINED)
		{
			m_Depth = VulkanBuffer::CreateImage2D(m_Specification.Width, m_Specification.Height, m_DepthFormat,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			views.push_back(m_Depth.View);
		}

		VkFramebufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = m_RenderPassClear;
		info.attachmentCount = (uint32_t)views.size();
		info.pAttachments = views.data();
		info.width = m_Specification.Width;
		info.height = m_Specification.Height;
		info.layers = 1;
		VulkanUtils::CheckVk(vkCreateFramebuffer(VulkanDevice::Get().GetDevice(), &info, nullptr, &m_Framebuffer), "vkCreateFramebuffer(fb)");
	}

	VulkanTargetInfo VulkanFramebuffer::GetTargetInfo() const
	{
		VulkanTargetInfo target;
		target.RenderPass = m_RenderPassClear;
		target.ColorAttachmentCount = (uint32_t)m_Colors.size();
		target.HasDepth = m_DepthFormat != VK_FORMAT_UNDEFINED;
		target.Extent = { m_Specification.Width, m_Specification.Height };
		target.FlipViewportY = false;
		return target;
	}

	void VulkanFramebuffer::RecordBeginPass(VkCommandBuffer cmd, VkRenderPass renderPass, const float clearColor[4])
	{
		std::vector<VkClearValue> clearValues;
		for (const auto& color : m_Colors)
		{
			VkClearValue value{};
			if (color.TextureFormat == FramebufferTextureFormat::RED_INTEGER)
				value.color.int32[0] = color.ClearInt;
			else
				memcpy(value.color.float32, clearColor, sizeof(float) * 4);
			clearValues.push_back(value);
		}
		if (m_DepthFormat != VK_FORMAT_UNDEFINED)
		{
			VkClearValue value{};
			value.depthStencil = { 1.0f, 0 };
			clearValues.push_back(value);
		}

		VkRenderPassBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = renderPass;
		info.framebuffer = m_Framebuffer;
		info.renderArea.extent = { m_Specification.Width, m_Specification.Height };
		info.clearValueCount = (uint32_t)clearValues.size();
		info.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanFramebuffer::BeginPass(VkCommandBuffer cmd, bool clear, const float clearColor[4])
	{
		if (m_PassActive)
			return;
		RecordBeginPass(cmd, clear ? m_RenderPassClear : m_RenderPassLoad, clearColor);
		m_PassActive = true;
	}

	void VulkanFramebuffer::EndPass(VkCommandBuffer cmd)
	{
		if (!m_PassActive)
			return;
		vkCmdEndRenderPass(cmd);
		m_PassActive = false;
	}

	void VulkanFramebuffer::RecordClear(VkCommandBuffer cmd, const float clearColor[4])
	{
		std::vector<VkClearAttachment> clears;
		for (uint32_t i = 0; i < m_Colors.size(); ++i)
		{
			if (m_Colors[i].TextureFormat == FramebufferTextureFormat::RED_INTEGER)
				continue;
			VkClearAttachment clear{};
			clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			clear.colorAttachment = i;
			memcpy(clear.clearValue.color.float32, clearColor, sizeof(float) * 4);
			clears.push_back(clear);
		}
		if (m_DepthFormat != VK_FORMAT_UNDEFINED)
		{
			VkClearAttachment clear{};
			clear.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			clear.clearValue.depthStencil = { 1.0f, 0 };
			clears.push_back(clear);
		}
		if (clears.empty())
			return;

		VkClearRect rect{};
		rect.rect.extent = { m_Specification.Width, m_Specification.Height };
		rect.layerCount = 1;
		vkCmdClearAttachments(cmd, (uint32_t)clears.size(), clears.data(), 1, &rect);
	}

	void VulkanFramebuffer::Bind()
	{
		if (auto* ctx = VulkanContext::Get())
			ctx->SetCurrentFramebuffer(this);
	}

	void VulkanFramebuffer::Unbind()
	{
		auto* ctx = VulkanContext::Get();
		if (!ctx)
			return;
		if (ctx->IsFrameActive())
			EndPass(ctx->GetCurrentCommandBuffer());
		if (ctx->GetCurrentFramebuffer() == this)
			ctx->SetCurrentFramebuffer(nullptr);
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			FE_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}
		auto* ctx = VulkanContext::Get();
		if (ctx && ctx->IsFrameActive() && m_PassActive)
			EndPass(ctx->GetCurrentCommandBuffer());

		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	void VulkanFramebuffer::RecordPixelCopy(VkCommandBuffer cmd, uint32_t attachmentIndex, int x, int y, uint32_t slot)
	{
		const VkImage image = m_Colors[attachmentIndex].Image.Image;
		VulkanBuffer::TransitionImageLayout(cmd, image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkBufferImageCopy region{};
		region.bufferOffset = sizeof(int32_t) * slot;
		region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		region.imageOffset = { x, y, 0 };
		region.imageExtent = { 1, 1, 1 };
		vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Readback.Buffer, 1, &region);

		VulkanBuffer::TransitionImageLayout(cmd, image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	// In-frame reads are asynchronous: the pixel copy is recorded into the frame and the value
	// returned is the one requested MaxFramesInFlight frames ago for this frame slot.
	// Outside a frame (setup / probes) the read is synchronous.
	int VulkanFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		FE_CORE_ASSERT(attachmentIndex < m_Colors.size(), "ReadPixel index out of range");
		if (x < 0 || y < 0 || x >= (int)m_Specification.Width || y >= (int)m_Specification.Height)
			return -1;

		auto* ctx = VulkanContext::Get();
		auto* readback = static_cast<volatile int32_t*>(m_Readback.Mapped);

		if (ctx && ctx->IsFrameActive())
		{
			const uint32_t slot = ctx->GetFrameIndex();
			const int previous = readback[slot];
			VkCommandBuffer cmd = ctx->GetCurrentCommandBuffer();
			EndPass(cmd);
			RecordPixelCopy(cmd, attachmentIndex, x, y, slot);
			return previous;
		}

		VulkanBuffer::ImmediateSubmit([&](VkCommandBuffer cmd)
		{
			RecordPixelCopy(cmd, attachmentIndex, x, y, kImmediateReadbackSlot);
		});
		return readback[kImmediateReadbackSlot];
	}

	void VulkanFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		FE_CORE_ASSERT(attachmentIndex < m_Colors.size(), "ClearAttachment index out of range");
		m_Colors[attachmentIndex].ClearInt = value;

		VkClearAttachment clear{};
		clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		clear.colorAttachment = attachmentIndex;
		if (m_Colors[attachmentIndex].TextureFormat == FramebufferTextureFormat::RED_INTEGER)
			clear.clearValue.color.int32[0] = value;

		VkClearRect rect{};
		rect.rect.extent = { m_Specification.Width, m_Specification.Height };
		rect.layerCount = 1;

		auto* ctx = VulkanContext::Get();
		const float black[4] = { 0, 0, 0, 0 };
		if (ctx && ctx->IsFrameActive())
		{
			if (ctx->GetCurrentFramebuffer() != this)
			{
				FE_CORE_WARN("Vulkan: ClearAttachment on a framebuffer that is not bound; ignored");
				return;
			}
			ctx->EnsureRenderPass();
			vkCmdClearAttachments(ctx->GetCurrentCommandBuffer(), 1, &clear, 1, &rect);
			return;
		}

		VulkanBuffer::ImmediateSubmit([&](VkCommandBuffer cmd)
		{
			BeginPass(cmd, false, black);
			vkCmdClearAttachments(cmd, 1, &clear, 1, &rect);
			EndPass(cmd);
		});
	}

	void* VulkanFramebuffer::GetColorAttachmentImGuiID(uint32_t index) const
	{
		if (index >= m_Colors.size() || !VulkanContext::IsImGuiInitialized())
			return nullptr;
		const auto& color = m_Colors[index];
		if (color.ImGuiSet == VK_NULL_HANDLE)
			color.ImGuiSet = ImGui_ImplVulkan_AddTexture(m_Sampler, color.Image.View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return (void*)color.ImGuiSet;
	}
}

#endif
