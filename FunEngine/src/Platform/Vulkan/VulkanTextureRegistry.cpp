#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanTextureRegistry.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

#include <algorithm>

namespace FunEngine
{
	VulkanTextureRegistry& VulkanTextureRegistry::Get()
	{
		static VulkanTextureRegistry instance;
		return instance;
	}

	void VulkanTextureRegistry::Init()
	{
		if (m_Initialized)
			return;

		auto& vulkanDevice = VulkanDevice::Get();
		VkDevice device = vulkanDevice.GetDevice();
		const auto& indexing = vulkanDevice.GetDescriptorIndexingSupport();
		const auto& limits = vulkanDevice.GetLimits();

		m_UsesIndexing = indexing.IsBindlessCapable();
		if (m_UsesIndexing)
		{
			m_MaxTextureSlots = std::min({ kMaxIndexedTextureSlots,
				indexing.MaxPerStageUpdateAfterBindSampledImages,
				indexing.MaxPerStageUpdateAfterBindSamplers });
			FE_CORE_INFO("Vulkan textures: descriptor indexing path (partially-bound, update-after-bind) with {0} slots", m_MaxTextureSlots);
		}
		else
		{
			m_MaxTextureSlots = std::min(kFixedTextureSlots, limits.maxPerStageDescriptorSamplers);
			FE_CORE_WARN("Vulkan textures: descriptor indexing unavailable, using fixed array of {0} slots", m_MaxTextureSlots);
		}

		VkDescriptorSetLayoutBinding bindings[2]{};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[1].descriptorCount = m_MaxTextureSlots;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorBindingFlags bindingFlags[2] = { 0, 0 };
		if (m_UsesIndexing)
			bindingFlags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
		flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		flagsInfo.bindingCount = 2;
		flagsInfo.pBindingFlags = bindingFlags;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 2;
		layoutInfo.pBindings = bindings;
		layoutInfo.pNext = &flagsInfo;
		if (m_UsesIndexing)
			layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		VulkanUtils::CheckVk(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_SetLayout), "vkCreateDescriptorSetLayout");

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_SetLayout;
		VulkanUtils::CheckVk(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout), "vkCreatePipelineLayout");

		VkDescriptorPoolSize poolSizes[2]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = kSetsPerFrame;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = kSetsPerFrame * m_MaxTextureSlots;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = kSetsPerFrame;
		poolInfo.poolSizeCount = 2;
		poolInfo.pPoolSizes = poolSizes;
		if (m_UsesIndexing)
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		for (auto& pool : m_Pools)
			VulkanUtils::CheckVk(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool), "vkCreateDescriptorPool");

		m_Slots.assign(m_MaxTextureSlots, TextureSlot{});

		m_FallbackUniform = VulkanBuffer::CreateHostBuffer(256, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		memset(m_FallbackUniform.Mapped, 0, 256);

		m_FallbackImage = VulkanBuffer::CreateImage2D(1, 1, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		{
			uint32_t white = 0xffffffff;
			VulkanBufferResource staging = VulkanBuffer::CreateHostBuffer(4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			memcpy(staging.Mapped, &white, 4);
			VulkanBuffer::ImmediateSubmit([&](VkCommandBuffer cmd)
			{
				VulkanBuffer::TransitionImageLayout(cmd, m_FallbackImage.Image, VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				VkBufferImageCopy region{};
				region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
				region.imageExtent = { 1, 1, 1 };
				vkCmdCopyBufferToImage(cmd, staging.Buffer, m_FallbackImage.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
				VulkanBuffer::TransitionImageLayout(cmd, m_FallbackImage.Image, VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			});
			VulkanBuffer::DestroyBuffer(staging);
		}

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.maxLod = 1.0f;
		VulkanUtils::CheckVk(vkCreateSampler(device, &samplerInfo, nullptr, &m_FallbackSampler), "vkCreateSampler(fallback)");

		m_Initialized = true;
	}

	void VulkanTextureRegistry::Shutdown()
	{
		if (!m_Initialized)
			return;

		VkDevice device = VulkanDevice::Get().GetDevice();
		if (device != VK_NULL_HANDLE)
		{
			for (auto& pool : m_Pools)
				vkDestroyDescriptorPool(device, pool, nullptr);
			vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
			vkDestroyDescriptorSetLayout(device, m_SetLayout, nullptr);
			vkDestroySampler(device, m_FallbackSampler, nullptr);
		}
		VulkanBuffer::DestroyBuffer(m_FallbackUniform);
		VulkanBuffer::DestroyImage(m_FallbackImage);

		m_Pools = {};
		m_PipelineLayout = VK_NULL_HANDLE;
		m_SetLayout = VK_NULL_HANDLE;
		m_FallbackSampler = VK_NULL_HANDLE;
		m_Slots.clear();
		m_CurrentSet = VK_NULL_HANDLE;
		m_UniformBuffer = VK_NULL_HANDLE;
		m_Initialized = false;
	}

	void VulkanTextureRegistry::BeginFrame(uint32_t frameIndex)
	{
		m_FrameIndex = frameIndex % (uint32_t)m_Pools.size();
		vkResetDescriptorPool(VulkanDevice::Get().GetDevice(), m_Pools[m_FrameIndex], 0);
		m_CurrentSet = VK_NULL_HANDLE;
		m_Dirty = true;
		m_PoolExhaustedWarned = false;
	}

	void VulkanTextureRegistry::SetUniformBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
	{
		if (binding != 0)
		{
			FE_CORE_WARN("Vulkan: uniform buffer binding {0} is not supported (only binding 0)", binding);
			return;
		}
		if (m_UniformBuffer == buffer && m_UniformOffset == offset && m_UniformRange == range)
			return;
		m_UniformBuffer = buffer;
		m_UniformOffset = offset;
		m_UniformRange = range;
		m_Dirty = true;
	}

	void VulkanTextureRegistry::SetTexture(uint32_t slot, VkImageView view, VkSampler sampler)
	{
		if (slot >= m_MaxTextureSlots)
		{
			FE_CORE_WARN("Vulkan: texture slot {0} out of range (max {1})", slot, m_MaxTextureSlots);
			return;
		}
		TextureSlot& entry = m_Slots[slot];
		if (entry.View == view && entry.Sampler == sampler)
			return;
		entry.View = view;
		entry.Sampler = sampler;
		m_HighestSlot = std::max(m_HighestSlot, slot);
		m_Dirty = true;
	}

	void VulkanTextureRegistry::OnTextureDestroyed(VkImageView view)
	{
		for (auto& entry : m_Slots)
		{
			if (entry.View == view)
				entry = TextureSlot{};
		}
		m_Dirty = true;
	}

	void VulkanTextureRegistry::OnUniformBufferDestroyed(VkBuffer buffer)
	{
		if (m_UniformBuffer == buffer)
		{
			m_UniformBuffer = VK_NULL_HANDLE;
			m_Dirty = true;
		}
	}

	VkDescriptorSet VulkanTextureRegistry::AcquireDescriptorSet()
	{
		if (!m_Dirty && m_CurrentSet != VK_NULL_HANDLE)
			return m_CurrentSet;

		VkDevice device = VulkanDevice::Get().GetDevice();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_Pools[m_FrameIndex];
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_SetLayout;

		VkDescriptorSet set = VK_NULL_HANDLE;
		if (vkAllocateDescriptorSets(device, &allocInfo, &set) != VK_SUCCESS)
		{
			if (!m_PoolExhaustedWarned)
			{
				FE_CORE_WARN("Vulkan: per-frame descriptor pool exhausted, reusing previous set");
				m_PoolExhaustedWarned = true;
			}
			return m_CurrentSet;
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffer != VK_NULL_HANDLE ? m_UniformBuffer : m_FallbackUniform.Buffer;
		bufferInfo.offset = m_UniformBuffer != VK_NULL_HANDLE ? m_UniformOffset : 0;
		bufferInfo.range = m_UniformBuffer != VK_NULL_HANDLE ? m_UniformRange : 256;

		std::vector<VkDescriptorImageInfo> imageInfos;
		const uint32_t count = m_UsesIndexing ? (m_HighestSlot + 1) : m_MaxTextureSlots;
		std::vector<VkWriteDescriptorSet> writes;
		writes.reserve(count + 1);

		VkWriteDescriptorSet bufferWrite{};
		bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWrite.dstSet = set;
		bufferWrite.dstBinding = 0;
		bufferWrite.descriptorCount = 1;
		bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bufferWrite.pBufferInfo = &bufferInfo;
		writes.push_back(bufferWrite);

		imageInfos.resize(count);
		for (uint32_t i = 0; i < count; ++i)
		{
			const TextureSlot& entry = m_Slots[i];
			const bool bound = entry.View != VK_NULL_HANDLE;
			if (!bound && m_UsesIndexing)
			{
				imageInfos[i] = {};
				continue;
			}
			imageInfos[i].imageView = bound ? entry.View : m_FallbackImage.View;
			imageInfos[i].sampler = bound ? entry.Sampler : m_FallbackSampler;
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = set;
			write.dstBinding = 1;
			write.dstArrayElement = i;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &imageInfos[i];
			writes.push_back(write);
		}

		vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);

		m_CurrentSet = set;
		m_Dirty = false;
		return set;
	}
}

#endif
