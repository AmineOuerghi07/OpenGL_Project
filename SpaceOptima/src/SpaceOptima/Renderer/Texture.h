#pragma once
#include <string>

#include "SpaceOptima/Core/Core.h"

namespace SpaceOptima
{
	class Texture
	{
	public:
		virtual ~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual const std::string& GetTexturePath() const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void SetData(void* data, size_t size) const = 0;
		virtual bool operator==(const Texture& other) const = 0;

		// this function is working only on OpenGL 4.6 and above and mostly on NVIDIA and AMD GPUs
		virtual void MakeTextureResident() const = 0;
		virtual uint64_t GetHandle() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);

		
	};
}