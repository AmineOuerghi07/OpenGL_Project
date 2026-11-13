#include "fepch.h"
#include <cstdio>
#include "ImGuiLayer.h"
#include "imgui.h"

#include "FunEngine/Core/Application.h"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#ifdef FE_WITH_VULKAN
#include "backends/imgui_impl_vulkan.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#endif

// TEMP
#include <GLFW/glfw3.h>
#include <glad/glad.h>


#include "ImGuizmo.h"
#include "FunEngine/Renderer/RendererAPI.h"

namespace FunEngine {

	namespace
	{
		static bool IsVulkan()
		{
			return RendererAPI::GetAPI() == RendererAPI::API::Vulkan;
		}

		static bool UseImGuiForCurrentRenderer()
		{
#ifdef FE_WITH_VULKAN
			if (IsVulkan())
				return true;
#endif
			return RendererAPI::GetAPI() == RendererAPI::API::OpenGL;
		}

#ifdef FE_WITH_VULKAN
		static VkDescriptorPool s_ImGuiDescriptorPool = VK_NULL_HANDLE;

		static void CheckImGuiVk(VkResult result)
		{
			if (result != VK_SUCCESS)
				FE_CORE_ERROR("ImGui Vulkan backend error: {0}", (int)result);
		}

		static void InitVulkanBackend(GLFWwindow* window)
		{
			auto* context = VulkanContext::Get();
			FE_CORE_ASSERT(context, "Vulkan context missing for ImGui");
			auto& device = VulkanDevice::Get();

			VkDescriptorPoolSize poolSizes[] = {
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			};
			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			poolInfo.maxSets = 1000;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes = poolSizes;
			CheckImGuiVk(vkCreateDescriptorPool(device.GetDevice(), &poolInfo, nullptr, &s_ImGuiDescriptorPool));

			ImGui_ImplGlfw_InitForVulkan(window, true);

			ImGui_ImplVulkan_InitInfo initInfo{};
			initInfo.Instance = device.GetInstance();
			initInfo.PhysicalDevice = device.GetPhysicalDevice();
			initInfo.Device = device.GetDevice();
			initInfo.QueueFamily = device.GetGraphicsQueueFamily();
			initInfo.Queue = device.GetGraphicsQueue();
			initInfo.DescriptorPool = s_ImGuiDescriptorPool;
			initInfo.RenderPass = context->GetRenderPass();
			initInfo.MinImageCount = 2;
			initInfo.ImageCount = std::max(2u, context->GetSwapchainImageCount());
			initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			initInfo.CheckVkResultFn = CheckImGuiVk;
			ImGui_ImplVulkan_Init(&initInfo);

			VulkanContext::SetImGuiInitialized(true);
		}
#endif
	}

	
	ImGuiLayer::ImGuiLayer() :Layer("ImGuiLayer") {

	}


	ImGuiLayer::~ImGuiLayer() {
	
	}

	void ImGuiLayer::OnAttach() {
		if (!UseImGuiForCurrentRenderer())
			return;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		// Enable Docking
		if (!IsVulkan())
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	// Multi-viewport is not implemented for the Vulkan backend

		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear Imgui fonts (Editor ships assets/fonts; Sandbox may not).
		{
			const char* boldPath = "assets/fonts/opensans/OpenSans-Bold.ttf";
			const char* regularPath = "assets/fonts/opensans/OpenSans-Regular.ttf";
			FILE* bold = fopen(boldPath, "rb");
			FILE* regular = fopen(regularPath, "rb");
			if (bold && regular)
			{
				fclose(bold);
				fclose(regular);
				io.Fonts->AddFontFromFileTTF(boldPath, 18.0f);
				io.FontDefault = io.Fonts->AddFontFromFileTTF(regularPath, 18.0f);
			}
			else
			{
				if (bold) fclose(bold);
				if (regular) fclose(regular);
				FE_CORE_WARN("ImGui fonts not found under assets/fonts — using default font");
				io.Fonts->AddFontDefault();
			}
		}

		ImGui::StyleColorsDark();
		// ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can lock identical to regular ones
		ImGuiStyle& style = ImGui::GetStyle();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColor();

		Application& app = Application::GetApplication();

		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings 
#ifdef FE_WITH_VULKAN
		if (IsVulkan())
		{
			InitVulkanBackend(window);
			m_Initialized = true;
			return;
		}
#endif
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
		m_Initialized = true;
	}

	void ImGuiLayer::Begin()
	{
		m_FrameBegun = false;
		if (!UseImGuiForCurrentRenderer() || !m_Initialized)
			return;

#ifdef FE_WITH_VULKAN
		if (IsVulkan())
		{
			auto* context = VulkanContext::Get();
			if (!context || !context->IsFrameActive())
				return;
			ImGui_ImplVulkan_NewFrame();
		}
		else
#endif
		ImGui_ImplOpenGL3_NewFrame();
		m_FrameBegun = true;
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		if (!m_FrameBegun)
			return;
		m_FrameBegun = false;

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::GetApplication();
		io.DisplaySize = ImVec2((float) app.GetWindow().GetWidth(), (float) app.GetWindow().GetHeight());

		// Rendering

		ImGui::Render();
#ifdef FE_WITH_VULKAN
		if (IsVulkan())
		{
			auto* context = VulkanContext::Get();
			context->SetCurrentFramebuffer(nullptr);
			if (context->EnsureRenderPass())
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), context->GetCurrentCommandBuffer());
			return;
		}
#endif
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	

	void ImGuiLayer::OnDetach() {
		if (!UseImGuiForCurrentRenderer() || !m_Initialized)
			return;
		m_Initialized = false;

#ifdef FE_WITH_VULKAN
		if (IsVulkan())
		{
			if (auto* context = VulkanContext::Get())
				context->WaitIdle();
			VulkanContext::SetImGuiInitialized(false);
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			if (s_ImGuiDescriptorPool != VK_NULL_HANDLE)
			{
				vkDestroyDescriptorPool(VulkanDevice::Get().GetDevice(), s_ImGuiDescriptorPool, nullptr);
				s_ImGuiDescriptorPool = VK_NULL_HANDLE;
			}
			return;
		}
#endif
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}


	void ImGuiLayer::SetDarkThemeColor()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}
	
}