#include "../include/application.hpp"

#include <chrono> // for animation

void Application::init() {
    std::cout << "Application init" << std::endl;

    // Window must be initialized before the core,
    // otherwise GLFW doens't give the correct list of required extensions,
    // which then leads to failing to initialize the surface.
    renderer.initWindow();

    core.init1();

    renderer.initSurface(core.getInstance());

    // The core doesn't own the surface, but it uses the Vulkan instance to initialize it.
    core.init2(renderer.getSurface());

    renderer.initRest(core);
    renderer.loadModels(modelPaths, texturePaths);

    camera.init(core, renderer.getInputListener(), renderer.getMaxFramesInFlight());

    std::cout << "renderer.updateDescriptorSets" << std::endl;
    renderer.updateDescriptorSets(camera.uniformBuffers);
}

void Application::run() {
    init();
    
    std::cout << "while loop" << std::endl;

    auto previousTime {std::chrono::high_resolution_clock::now()};
    auto currentTime {previousTime}; // assign to previousTime just for auto to work
    float deltaTime;

    while (renderer.step()) {
        currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();

        camera.updateUniformBuffer(
            renderer.getFrameIndex(),
            renderer.getSwapChainExtentWidth(),
            renderer.getSwapChainExtentHeight(),
            deltaTime
        );
        
        renderer.drawFrame(deltaTime);

        previousTime = currentTime;
    }

    renderer.cleanup();
}