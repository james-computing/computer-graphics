#include "../include/application.hpp"

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

    camera.init(core, renderer.getMaxFramesInFlight());
    
    std::cout << "model load" << std::endl;
    model.load(core, renderer, modelPath, texturePath);

    /*
    std::cout << "create object" << std::endl;
    object.init(core, renderer.getMaxFramesInFlight());
    */

    std::cout << "create model instances" << std::endl;
    modelInstances.init(core, renderer.getMaxFramesInFlight());

    std::cout << "renderer.updateDescriptorSets" << std::endl;
    renderer.updateDescriptorSets(
        model.texture.imageView,
        camera.uniformBuffers,
        modelInstances.shaderStorageBuffers,
        modelInstances.getInstanceCount()
    );
}

void Application::run() {
    init();
    
    std::cout << "while loop" << std::endl;
    while (renderer.step()) {
        camera.updateUniformBuffer(renderer.getFrameIndex(), renderer.getSwapChainExtentWidth(), renderer.getSwapChainExtentHeight());
        //object.updateUniformBuffer(renderer.getFrameIndex());
        modelInstances.updateShaderStorageBuffer(renderer.getFrameIndex());
        renderer.drawFrame(model.getNumIndices(), modelInstances.getInstanceCount());
    }

    renderer.cleanup();
}