#pragma once

#ifdef FE_WITH_VULKAN

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include "VulkanBuffer.h"

namespace FunEngine
{
	// Global descriptor model shared by every FunEngine Vulkan pipeline (set 0):
	//   binding 0 : uniform buffer (Camera)
	//   binding 1 : combined image sampler array (bindless-style when descriptor indexing is available,
	//               otherwise a fixed array of 16 like the macOS OpenGL path)
	// A fresh descriptor set is written per state change from a per-frame pool, so batches
	// recorded earlier in the frame never see later texture/UBO updates.
	class VulkanTextureRegistry
	{
	public:
		static constexpr uint32_t kFixedTextureSlots = 16;
		static constexpr uint32_t kMaxIndexedTextureSlots = 1024;
		static constexpr uint32_t kSetsPerFrame = 64;

		static VulkanTextureRegistry& Get();

		void Init();
		void Shutdown();
		bool IsInitialized() const { return m_Initialized; }

		void BeginFrame(uint32_t frameIndex);

		bool UsesDescriptorIndexing() const { return m_UsesIndexing; }
		uint32_t GetMaxTextureSlots() const { return m_MaxTextureSlots; }
		VkDescriptorSetLayout GetSetLayout() const { return m_SetLayout; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

		void SetUniformBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
		void SetTexture(uint32_t slot, VkImageView view, VkSampler sampler);
		void OnTextureDestroyed(VkImageView view);
		void OnUniformBufferDestroyed(VkBuffer buffer);

		VkDescriptorSet AcquireDescriptorSet();

	private:
		struct TextureSlot
		{
			VkImageView View = VK_NULL_HANDLE;
			VkSampler Sampler = VK_NULL_HANDLE;
		};

		bool m_Initialized = false;
		bool m_UsesIndexing = false;
		uint32_t m_MaxTextureSlots = kFixedTextureSlots;

		VkDescriptorSetLayout m_SetLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		std::array<VkDescriptorPool, 2> m_Pools{};
		uint32_t m_FrameIndex = 0;

		std::vector<TextureSlot> m_Slots;
		uint32_t m_HighestSlot = 0;

		VkBuffer m_UniformBuffer = VK_NULL_HANDLE;
		VkDeviceSize m_UniformOffset = 0;
		VkDeviceSize m_UniformRange = 0;

		bool m_Dirty = true;
		VkDescriptorSet m_CurrentSet = VK_NULL_HANDLE;
		bool m_PoolExhaustedWarned = false;

		VulkanBufferResource m_FallbackUniform;
		VulkanImageResource m_FallbackImage;
		VkSampler m_FallbackSampler = VK_NULL_HANDLE;
	};
}

#endif
