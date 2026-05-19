#include "../include/surface.hpp"

void Surface::create(vk::raii::Instance const & instance, GLFWwindow * const glfwWindow) {
    // C struct
    VkSurfaceKHR _surface;
    // C function call
    VkResult result = glfwCreateWindowSurface(*instance, glfwWindow, nullptr, &_surface);

    if (result != VkResult::VK_SUCCESS) {
        std::cerr << "Failed to create window surface";
        return;
    }

    // Get a C++ surface from the C _surface
    vkraii = vk::raii::SurfaceKHR(instance, _surface);
}