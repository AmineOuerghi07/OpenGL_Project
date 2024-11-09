#pragma once
#include "SpaceOptima/Core/Core.h"

namespace SpaceOptima
{
	struct FramebufferSpecification
	{
		uint32_t Width, Height;
		//FramebufferFormat Format = 
		uint32_t Sample = 1;

		bool swapChaininTarget = false;

	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		virtual uint32_t GetColorAttachementRendererID() const = 0;
		
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;


		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	};
}