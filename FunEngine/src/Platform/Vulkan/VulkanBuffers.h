#pragma once

#ifdef FE_WITH_VULKAN

#include "FunEngine/Renderer/Buffer.h"
#include "FunEngine/Renderer/VertexArray.h"
#include "FunEngine/Renderer/UniformBuffer.h"
#include "FunEngine/Renderer/StorageBuffer.h"
#include "VulkanBuffer.h"

namespace FunEngine
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const float* vertices, uint32_t size);
		VulkanVertexBuffer(uint32_t size);
		~VulkanVertexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		BufferLayout GetLayout() const override { return m_Layout; }
		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		void SetData(const void* data, uint32_t size) override;

		VkBuffer GetBuffer() const { return m_Resource.Buffer; }
		VkDeviceSize GetBindOffset() const { return m_BindOffset; }

		static const VulkanVertexBuffer* GetBound();

	private:
		static constexpr uint32_t kRingRegions = 4;

		VulkanBufferResource m_Resource;
		BufferLayout m_Layout;
		uint32_t m_Size = 0;
		bool m_Dynamic = false;

		VkDeviceSize m_RegionSize = 0;
		VkDeviceSize m_Cursor = 0;
		uint64_t m_CursorFrameSerial = UINT64_MAX;
		VkDeviceSize m_BindOffset = 0;
		bool m_WrapWarned = false;
	};

	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const uint32_t* indices, uint32_t count);
		~VulkanIndexBuffer() override;

		void Bind() const override;
		void Unbind() const override;
		uint32_t GetCount() const override { return m_Count; }

		VkBuffer GetBuffer() const { return m_Resource.Buffer; }

		static const VulkanIndexBuffer* GetBound();

	private:
		VulkanBufferResource m_Resource;
		uint32_t m_Count = 0;
	};

	class VulkanVertexArray : public VertexArray
	{
	public:
		VulkanVertexArray() = default;
		~VulkanVertexArray() override = default;

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		const std::vector<Ref<VertexBuffer>>& GetVertexBuffer() const override { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

		static const VulkanVertexArray* GetBound();

	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};

	// Ring of aligned slots per frame in flight so earlier draws in a frame keep their own contents.
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		~VulkanUniformBuffer() override;

		void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	private:
		static constexpr uint32_t kSlotsPerFrame = 128;

		VulkanBufferResource m_Resource;
		std::vector<uint8_t> m_Shadow;
		uint32_t m_Size = 0;
		uint32_t m_Binding = 0;
		VkDeviceSize m_SlotStride = 0;
		uint32_t m_Cursor = 0;
		uint64_t m_CursorFrameSerial = UINT64_MAX;
		bool m_WrapWarned = false;
	};

	// R2D on Vulkan indexes textures through descriptors instead of GL bindless SSBOs.
	class VulkanStorageBuffer : public StorageBuffer
	{
	public:
		VulkanStorageBuffer(uint64_t size, uint32_t binding);
		void SetData(const void* data, uint32_t size, uint32_t offset) override;

	private:
		uint64_t m_Size = 0;
		uint32_t m_Binding = 0;
	};
}

#endif
