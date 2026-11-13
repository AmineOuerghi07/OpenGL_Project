#pragma once

#ifdef FE_WITH_VULKAN

#include "FunEngine/Renderer/Texture.h"
#include "VulkanBuffer.h"

namespace FunEngine
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(uint32_t width, uint32_t height);
		VulkanTexture2D(const std::string& path);
		~VulkanTexture2D() override;

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }
		uint32_t GetRendererID() const override { return m_ID; }
		const std::string& GetTexturePath() const override { return m_Path; }

		// Writes this texture into descriptor array element `slot` of the global texture set.
		void Bind(uint32_t slot = 0) const override;
		void SetData(void* data, size_t size) const override;

		bool operator==(const Texture& other) const override
		{
			return m_ID == static_cast<const VulkanTexture2D&>(other).m_ID;
		}

		// Stable, non-zero identifier (unique per texture) usable as a bindless index key.
		uint64_t GetHandle() const override { return m_ID; }
		void* GetImGuiID() const override;

		VkImageView GetImageView() const { return m_Image.View; }
		VkSampler GetSampler() const { return m_Sampler; }

	private:
		void Create();
		void Upload(const void* data, size_t size) const;

		std::string m_Path;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		uint32_t m_ID = 0;

		VulkanImageResource m_Image;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		mutable VkDescriptorSet m_ImGuiSet = VK_NULL_HANDLE;
	};
}

#endif
