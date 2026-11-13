#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanTexture.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanTextureRegistry.h"
#include "VulkanUtils.h"

#include "stb_image.h"
#include "backends/imgui_impl_vulkan.h"

#include <cstring>

namespace FunEngine
{
	namespace
	{
		static uint32_t s_NextTextureID = 1;
	}

	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height), m_ID(s_NextTextureID++)
	{
		Create();
	}

	VulkanTexture2D::VulkanTexture2D(const std::string& path)
		: m_Path(path), m_ID(s_NextTextureID++)
	{
		int width = 0, height = 0, channels = 0;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
		FE_CORE_ASSERT(data, "Failed to load image");
		m_Width = (uint32_t)width;
		m_Height = (uint32_t)height;

		Create();
		Upload(data, (size_t)width * height * 4);
		stbi_image_free(data);
	}

	void VulkanTexture2D::Create()
	{
		VkDevice device = VulkanDevice::Get().GetDevice();

		m_Image = VulkanBuffer::CreateImage2D(m_Width, m_Height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Matches OpenGLTexture2D: linear minification, nearest magnification, repeat.
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.maxLod = 1.0f;
		VulkanUtils::CheckVk(vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler), "vkCreateSampler");
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		if (VulkanDevice::Get().GetDevice() == VK_NULL_HANDLE)
			return;

		if (VulkanTextureRegistry::Get().IsInitialized())
			VulkanTextureRegistry::Get().OnTextureDestroyed(m_Image.View);

		VulkanImageResource image = m_Image;
		VkSampler sampler = m_Sampler;
		VkDescriptorSet imguiSet = m_ImGuiSet;
		auto destroy = [image, sampler, imguiSet]() mutable
		{
			if (imguiSet != VK_NULL_HANDLE && VulkanContext::IsImGuiInitialized())
				ImGui_ImplVulkan_RemoveTexture(imguiSet);
			VkDevice device = VulkanDevice::Get().GetDevice();
			if (device == VK_NULL_HANDLE)
				return;
			vkDestroySampler(device, sampler, nullptr);
			VulkanBuffer::DestroyImage(image);
		};
		if (auto* ctx = VulkanContext::Get())
			ctx->DeferDestroy(destroy);
		else
			destroy();
	}

	void VulkanTexture2D::Bind(uint32_t slot) const
	{
		VulkanTextureRegistry::Get().SetTexture(slot, m_Image.View, m_Sampler);
	}

	void VulkanTexture2D::Upload(const void* data, size_t size) const
	{
		VulkanBufferResource staging = VulkanBuffer::CreateHostBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		memcpy(staging.Mapped, data, size);

		VulkanBuffer::ImmediateSubmit([&](VkCommandBuffer cmd)
		{
			VulkanBuffer::TransitionImageLayout(cmd, m_Image.Image, VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkBufferImageCopy region{};
			region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			region.imageExtent = { m_Width, m_Height, 1 };
			vkCmdCopyBufferToImage(cmd, staging.Buffer, m_Image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			VulkanBuffer::TransitionImageLayout(cmd, m_Image.Image, VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});
		VulkanBuffer::DestroyBuffer(staging);
	}

	void VulkanTexture2D::SetData(void* data, size_t size) const
	{
		FE_CORE_ASSERT(size == (size_t)m_Width * m_Height * 4, "Size is not right !!!");
		Upload(data, size);
	}

	void* VulkanTexture2D::GetImGuiID() const
	{
		if (m_ImGuiSet == VK_NULL_HANDLE && VulkanContext::IsImGuiInitialized())
			m_ImGuiSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_Image.View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return (void*)m_ImGuiSet;
	}
}

#endif
