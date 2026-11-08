#pragma once
#include "FunEngine/Core/Core.h"

namespace FunEngine {
	class StorageBuffer
	{
	public:
		virtual ~StorageBuffer() {}

		virtual void SetData(const void* data, uint32_t size, uint32_t offset) = 0;

		static Ref<StorageBuffer> Create(uint64_t size, uint32_t binding);
	};
}