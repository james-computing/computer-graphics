#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "iinputListener.hpp"

class Window : public IInputListener{
public:
    uint32_t const WIDTH {800};
    uint32_t const HEIGHT {600};
    GLFWwindow * glfw {nullptr};

private:
    bool frameBufferResized {false};

public:
    void init();
    void getFramebufferSize(int & width, int & height) const;
    bool getFrameBufferResized() const;
    bool shouldClose() const;
    void pollEvents() const;
    void cleanup() const;
    
private:
    static void frameBufferResizeCallback(GLFWwindow * glfwWindow, int width, int height);

    static void keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);

    KeysActive keysActive;

public:
    bool getKeyActive(char const c) const override;
};