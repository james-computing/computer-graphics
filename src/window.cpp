#include "../include/window.hpp"

#include <iostream>

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

    glfwSetKeyCallback(glfw, keyCallback);
}

void Window::frameBufferResizeCallback(GLFWwindow * glfwWindow, int width, int height) {
    Window * const window {reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow))};
    window->frameBufferResized = true;
}

void Window::getFramebufferSize(int & width, int & height) const {
    glfwGetFramebufferSize(glfw, &width, &height);
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

void Window::cleanup() const {
    glfwDestroyWindow(glfw);
    glfwTerminate();
}

void Window::keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods) {
    Window * const windowPtr {reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow))};
    if (action == GLFW_PRESS) {
        switch(key) {
        case GLFW_KEY_Q:
            windowPtr->keysActive.q = true;
            break;
        case GLFW_KEY_W:
            windowPtr->keysActive.w = true;
            break;
        case GLFW_KEY_E:
            windowPtr->keysActive.e = true;
            break;
        case GLFW_KEY_A:
            windowPtr->keysActive.a = true;
            break;
        case GLFW_KEY_S:
            windowPtr->keysActive.s = true;
            break;
        case GLFW_KEY_D:
            windowPtr->keysActive.d = true;
            break;
        }
    } else if (action == GLFW_RELEASE) {
        switch(key) {
        case GLFW_KEY_Q:
            windowPtr->keysActive.q = false;
            break;
        case GLFW_KEY_W:
            windowPtr->keysActive.w = false;
            break;
        case GLFW_KEY_E:
            windowPtr->keysActive.e = false;
            break;
        case GLFW_KEY_A:
            windowPtr->keysActive.a = false;
            break;
        case GLFW_KEY_S:
            windowPtr->keysActive.s = false;
            break;
        case GLFW_KEY_D:
            windowPtr->keysActive.d = false;
            break;
        }
    }
}

bool Window::getKeyActive(char const c) const {
    switch (c)
    {
    case 'q':
        return keysActive.q;
    case 'w':
        return keysActive.w;
    case 'e':
        return keysActive.e;
    case 'a':
        return keysActive.a;
    case 's':
        return keysActive.s;
    case 'd':
        return keysActive.d;
    default:
        throw std::runtime_error("Checking for invalid key in Window::getKeyActive");
    }
}