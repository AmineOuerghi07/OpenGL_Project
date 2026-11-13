#ifdef FE_WITH_VULKAN

#include "fepch.h"
#include "VulkanRendererAPI.h"
#include "VulkanContext.h"
#include "FunEngine/Core/Application.h"
#include "FunEngine/Core/Window.h"

namespace FunEngine
{
	void VulkanRendererAPI::Init()
	{
		FE_CORE_INFO("VulkanRendererAPI::Init");
	}

	void VulkanRendererAPI::SetViewPort(uint32_t, uint32_t, uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return;

		auto* ctx = Application::GetApplication().GetWindow().GetGraphicsContext();
		if (!ctx)
			return;

		static_cast<VulkanContext*>(ctx)->SetFramebufferResized(true);
	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
		auto* ctx = Application::GetApplication().GetWindow().GetGraphicsContext();
		if (ctx)
			static_cast<VulkanContext*>(ctx)->SetClearColor(color);
	}

	void VulkanRendererAPI::Clear()
	{
		// Cleared during VulkanContext::SwapBuffers render pass.
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>&, uint32_t indexCount)
	{
		if (indexCount == 0)
			return;
		static bool warned = false;
		if (!warned)
		{
			FE_CORE_WARN("VulkanRendererAPI::DrawIndexed is not implemented yet");
			warned = true;
		}
	}
}

#endif
