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

    std::vector<std::string_view> const modelPaths {"./models/viking_room.obj", "./models/box_01.obj"};
    std::vector<std::string_view> const texturePaths {"./textures/viking_room.png", "./textures/tex_box_01_d.jpg"};
    std::vector<bool> const alphaCuts {true, false};
    renderer.loadModels(modelPaths, texturePaths, alphaCuts);

    camera.init(core, renderer.getInputListener(), renderer.getMaxFramesInFlight());

    std::cout << "renderer.updateDescriptorSets" << std::endl;
    renderer.updateDescriptorSets(camera.uniformBuffers);
}

void Application::run() {
    init();
    
    std::cout << "while loop" << std::endl;

    auto previousTime {std::chrono::steady_clock::now()};
    auto currentTime {previousTime}; // assign to previousTime just for auto to work
    float deltaTime;

    while (renderer.step()) {
        currentTime = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();
        camera.updateUniformBuffer(
            renderer.getFrameIndex(),
            renderer.getSwapChainExtentWidth(),
            renderer.getSwapChainExtentHeight(),
            deltaTime
        );
        
        renderer.drawFrame(deltaTime);

        previousTime = currentTime;
        // FPS
        //std::cout << 1.0f/deltaTime << '\n';
    }

    renderer.cleanup();
}