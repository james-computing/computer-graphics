#include "../include/window.hpp"

#include <iostream>

void Window::init() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't create an OpenGL context, since we're using Vulkan
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create a window.
    // The 4th parameter is to specify a monitor,
    // The 5th is for OpenGL.
    glfwWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetFramebufferSizeCallback(glfwWindow, frameBufferResizeCallback);

    glfwSetKeyCallback(glfwWindow, keyCallback);

    // Use raw mouse motion for camera rotation
    // Not working correctly?
    /*
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(glfwWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        std::cout << "GLFW_RAW_MOUSE_MOTION = TRUE" << std::endl;
    } else {
        std::cout << "GLFW_RAW_MOUSE_MOTION = FALSE" << std::endl;
    }
    
    glfwSetCursorPosCallback(glfwWindow, cursorPositionCallback);
    //glfwSetWindowFocusCallback(glfwWindow, windowFocusCallback);
    glfwSetCursorEnterCallback(glfwWindow, cursorEnterCallBack);
    */
}

void Window::frameBufferResizeCallback(GLFWwindow * glfwWindow, int width, int height) {
    Window * const window {reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow))};
    window->frameBufferResized = true;
}

void Window::getFramebufferSize(int & width, int & height) const {
    glfwGetFramebufferSize(glfwWindow, &width, &height);
}

bool Window::getFrameBufferResized() const {
    return frameBufferResized;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(glfwWindow);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

void Window::cleanup() const {
    glfwDestroyWindow(glfwWindow);
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
        // shift
        case GLFW_KEY_LEFT_SHIFT:
            windowPtr->keysActive.shift = true;
            break;
        // arrow keys
        case GLFW_KEY_UP:
            windowPtr->keysActive.up = true;
            break;
        case GLFW_KEY_DOWN:
            windowPtr->keysActive.down = true;
            break;
        case GLFW_KEY_LEFT:
            windowPtr->keysActive.left = true;
            break;
        case GLFW_KEY_RIGHT:
            windowPtr->keysActive.right = true;
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
        // shift
        case GLFW_KEY_LEFT_SHIFT:
            windowPtr->keysActive.shift = false;
            break;
        // arrow keys
        case GLFW_KEY_UP:
            windowPtr->keysActive.up = false;
            break;
        case GLFW_KEY_DOWN:
            windowPtr->keysActive.down = false;
            break;
        case GLFW_KEY_LEFT:
            windowPtr->keysActive.left = false;
            break;
        case GLFW_KEY_RIGHT:
            windowPtr->keysActive.right = false;
            break;
        }
    }
}

KeysActive const & Window::getKeysActive() const {
    return keysActive;
}

/*
void Window::cursorPositionCallback(GLFWwindow* glfwWindow, double xpos, double ypos) {
    Window * const windowPtr {reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow))};
    
    //std::cout << "(x,y) = (" << xpos << "," << ypos << ")" << std::endl;
    
    windowPtr->previousRawMouseInput.xpos = windowPtr->currentRawMouseInput.xpos;
    windowPtr->previousRawMouseInput.ypos = windowPtr->currentRawMouseInput.ypos;

    windowPtr->currentRawMouseInput.xpos = xpos;
    windowPtr->currentRawMouseInput.ypos = ypos;

    windowPtr->mouseInput.dx = windowPtr->currentRawMouseInput.xpos - windowPtr->previousRawMouseInput.xpos;
    windowPtr->mouseInput.dy = windowPtr->currentRawMouseInput.ypos - windowPtr->previousRawMouseInput.ypos;

    
}

MouseInput const & Window::getMouseInput() const {
    return mouseInput;
}

void Window::resetMouseInput() {
    previousRawMouseInput = currentRawMouseInput;
    mouseInput.dx = 0;
    mouseInput.dy = 0;
}

void Window::cursorEnterCallBack(GLFWwindow* glfwWindow, int entered) {
    Window * const windowPtr {reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow))};

    if(entered) {
        std::cout << "entered" << std::endl;
        windowPtr->entered = true;
    } else {
        std::cout << "exited" << std::endl;
        windowPtr->entered = false;
    }
    windowPtr->resetMouseInput();
}

void Window::windowFocusCallback(GLFWwindow* glfwWindow, int focused) {
    Window * const windowPtr {reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow))};

    if(focused) {
        windowPtr->focused = true;
        //windowPtr->resetRawMouseInput();
        std::cout << "focused" << std::endl;
    } else {
        std::cout << "unfocused" << std::endl;
        windowPtr->focused = false;
        windowPtr->mouseInput.dx = 0;
        windowPtr->mouseInput.dy = 0;
    }
}

bool Window::isWindowFocused() const {
    return focused;
}
*/

