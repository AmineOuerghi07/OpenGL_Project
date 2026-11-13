#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanBuffers.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanTextureRegistry.h"

#include <cstring>

namespace FunEngine
{
	namespace
	{
		static const VulkanVertexBuffer* s_BoundVertexBuffer = nullptr;
		static const VulkanIndexBuffer* s_BoundIndexBuffer = nullptr;
		static const VulkanVertexArray* s_BoundVertexArray = nullptr;

		static VkDeviceSize AlignUp(VkDeviceSize value, VkDeviceSize alignment)
		{
			return (value + alignment - 1) & ~(alignment - 1);
		}

		static uint32_t CurrentFrameIndex()
		{
			auto* ctx = VulkanContext::Get();
			return ctx ? ctx->GetFrameIndex() : 0;
		}

		static uint64_t CurrentFrameSerial()
		{
			auto* ctx = VulkanContext::Get();
			return ctx ? ctx->GetFrameSerial() : 0;
		}
	}

	// ---- Vertex buffer -------------------------------------------------------------------------

	VulkanVertexBuffer::VulkanVertexBuffer(const float* vertices, uint32_t size)
		: m_Size(size), m_Dynamic(false)
	{
		m_Resource = VulkanBuffer::CreateDeviceLocalBuffer(vertices, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
		: m_Size(size), m_Dynamic(true)
	{
		m_RegionSize = AlignUp((VkDeviceSize)size * kRingRegions, 16);
		m_Resource = VulkanBuffer::CreateHostBuffer(m_RegionSize * VulkanContext::MaxFramesInFlight, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		memset(m_Resource.Mapped, 0, (size_t)m_Resource.Size);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		if (s_BoundVertexBuffer == this)
			s_BoundVertexBuffer = nullptr;
		VulkanBuffer::DeferredDestroyBuffer(m_Resource);
	}

	void VulkanVertexBuffer::Bind() const { s_BoundVertexBuffer = this; }
	void VulkanVertexBuffer::Unbind() const
	{
		if (s_BoundVertexBuffer == this)
			s_BoundVertexBuffer = nullptr;
	}

	const VulkanVertexBuffer* VulkanVertexBuffer::GetBound() { return s_BoundVertexBuffer; }

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
	{
		FE_CORE_ASSERT(m_Dynamic, "SetData on a static Vulkan vertex buffer");
		if (size == 0)
			return;
		FE_CORE_ASSERT(size <= m_Size, "Vulkan vertex buffer overflow");

		const uint64_t serial = CurrentFrameSerial();
		if (serial != m_CursorFrameSerial)
		{
			m_CursorFrameSerial = serial;
			m_Cursor = 0;
		}

		const VkDeviceSize aligned = AlignUp(size, 16);
		if (m_Cursor + aligned > m_RegionSize)
		{
			if (!m_WrapWarned)
			{
				FE_CORE_WARN("Vulkan: dynamic vertex buffer ring wrapped within a frame (earlier batches may be overwritten)");
				m_WrapWarned = true;
			}
			m_Cursor = 0;
		}

		m_BindOffset = (VkDeviceSize)CurrentFrameIndex() * m_RegionSize + m_Cursor;
		memcpy(static_cast<uint8_t*>(m_Resource.Mapped) + m_BindOffset, data, size);
		m_Cursor += aligned;
	}

	// ---- Index buffer --------------------------------------------------------------------------

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		m_Resource = VulkanBuffer::CreateDeviceLocalBuffer(indices, (VkDeviceSize)count * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		if (s_BoundIndexBuffer == this)
			s_BoundIndexBuffer = nullptr;
		VulkanBuffer::DeferredDestroyBuffer(m_Resource);
	}

	void VulkanIndexBuffer::Bind() const { s_BoundIndexBuffer = this; }
	void VulkanIndexBuffer::Unbind() const
	{
		if (s_BoundIndexBuffer == this)
			s_BoundIndexBuffer = nullptr;
	}

	const VulkanIndexBuffer* VulkanIndexBuffer::GetBound() { return s_BoundIndexBuffer; }

	// ---- Vertex array --------------------------------------------------------------------------

	void VulkanVertexArray::Bind() const { s_BoundVertexArray = this; }
	void VulkanVertexArray::Unbind() const
	{
		if (s_BoundVertexArray == this)
			s_BoundVertexArray = nullptr;
	}

	const VulkanVertexArray* VulkanVertexArray::GetBound() { return s_BoundVertexArray; }

	void VulkanVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		FE_CORE_ASSERT(vertexBuffer->GetLayout().GetStride(), "Vertex buffer has no layout");
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void VulkanVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
	}

	// ---- Uniform buffer ------------------------------------------------------------------------

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
		: m_Size(size), m_Binding(binding)
	{
		const VkDeviceSize alignment = std::max<VkDeviceSize>(16, VulkanDevice::Get().GetLimits().minUniformBufferOffsetAlignment);
		m_SlotStride = AlignUp(size, alignment);
		m_Resource = VulkanBuffer::CreateHostBuffer(m_SlotStride * kSlotsPerFrame * VulkanContext::MaxFramesInFlight,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		memset(m_Resource.Mapped, 0, (size_t)m_Resource.Size);
		m_Shadow.assign(size, 0);
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		if (VulkanTextureRegistry::Get().IsInitialized())
			VulkanTextureRegistry::Get().OnUniformBufferDestroyed(m_Resource.Buffer);
		VulkanBuffer::DeferredDestroyBuffer(m_Resource);
	}

	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		FE_CORE_ASSERT(offset + size <= m_Size, "Vulkan uniform buffer overflow");
		memcpy(m_Shadow.data() + offset, data, size);

		const uint64_t serial = CurrentFrameSerial();
		if (serial != m_CursorFrameSerial)
		{
			m_CursorFrameSerial = serial;
			m_Cursor = 0;
		}
		if (m_Cursor >= kSlotsPerFrame)
		{
			if (!m_WrapWarned)
			{
				FE_CORE_WARN("Vulkan: uniform buffer ring wrapped within a frame");
				m_WrapWarned = true;
			}
			m_Cursor = 0;
		}

		const VkDeviceSize slotOffset = ((VkDeviceSize)CurrentFrameIndex() * kSlotsPerFrame + m_Cursor) * m_SlotStride;
		memcpy(static_cast<uint8_t*>(m_Resource.Mapped) + slotOffset, m_Shadow.data(), m_Size);
		m_Cursor++;

		VulkanTextureRegistry::Get().SetUniformBuffer(m_Binding, m_Resource.Buffer, slotOffset, m_Size);
	}

	// ---- Storage buffer ------------------------------------------------------------------------

	VulkanStorageBuffer::VulkanStorageBuffer(uint64_t size, uint32_t binding)
		: m_Size(size), m_Binding(binding)
	{
	}

	void VulkanStorageBuffer::SetData(const void*, uint32_t, uint32_t)
	{
		// Intentionally empty: Vulkan Renderer2D indexes textures through descriptor arrays.
	}
}

#endif
