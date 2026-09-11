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
/*
struct RawMouseInput {
    double xpos;
    double ypos;
};
*/

class Window : public IInputListener{
public:
    uint32_t const WIDTH {800};
    uint32_t const HEIGHT {600};
    GLFWwindow * glfwWindow {nullptr};

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

    KeysActive keysActive;
    static void keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
/*
    // Mouse not working correctly?
    MouseInput mouseInput;
    RawMouseInput previousRawMouseInput;
    RawMouseInput currentRawMouseInput;
    static void cursorPositionCallback(GLFWwindow* glfwWindow, double xpos, double ypos);

    bool entered;
    static void cursorEnterCallBack(GLFWwindow* glfwWindow, int entered);

    bool focused;
    static void windowFocusCallback(GLFWwindow* glfwWindow, int focused);

    void resetMouseInput();
*/

public:
    KeysActive const & getKeysActive() const override;
    //MouseInput const & getMouseInput() const override;
    //bool isWindowFocused() const;
};