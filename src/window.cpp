#include "../include/window.hpp"

void Window::init() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't create an OpenGL context, since we're using Vulkan
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create a window.
    // The 4th parameter is to specify a monitor,
    // The 5th is for OpenGL.
    glfw = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(glfw, this);
    glfwSetFramebufferSizeCallback(glfw, frameBufferResizeCallback);
}

void Window::frameBufferResizeCallback(GLFWwindow * glfwWindow, int width, int height) {
    Window * const window {reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow))};
    window->frameBufferResized = true;
}

bool Window::getFrameBufferResized() const {
    return frameBufferResized;    
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(glfw);
}

void Window::pollEvents() const {
    glfwPollEvents();
}