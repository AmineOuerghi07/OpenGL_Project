#include "fepch.h"

#ifdef FE_WITH_VULKAN
// Must precede the GLFW backend so it does not forward-declare VkResult itself.
#include <vulkan/vulkan.h>
#endif

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include "backends/imgui_impl_opengl3.cpp"

#include "backends/imgui_impl_glfw.cpp"

#ifdef FE_WITH_VULKAN
#include "backends/imgui_impl_vulkan.cpp"
#endif
