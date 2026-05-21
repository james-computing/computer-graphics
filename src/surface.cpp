#include "../include/surface.hpp"

#include <vulkan/vk_enum_string_helper.h>

void Surface::create(vk::raii::Instance const & instance, GLFWwindow * const glfwWindow) {
    // C struct
    VkSurfaceKHR _surface;
    // C function call
    VkResult result = glfwCreateWindowSurface(*instance, glfwWindow, nullptr, &_surface);

    if (result != VkResult::VK_SUCCESS) {
        std::cerr << string_VkResult(result) << std::endl;
        throw std::runtime_error("Failed to create window surface");
    }

    // Get a C++ surface from the C _surface
    vkraii = vk::raii::SurfaceKHR(instance, _surface);
}