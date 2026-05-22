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

    std::cout << "Create texture sampler" << std::endl;
    // depends on the logical and physical devices.
    // Used in createDescriptorSets.
    TextureSampler::create(core.getPhysicalDevice(), core.getDevice(), textureSampler);
    
    std::cout << "model load" << std::endl;
    model.load(core, renderer, modelPath, texturePath);

    std::cout << "create object" << std::endl;
    object.init(model, core, renderer, textureSampler);
}

void Application::run() {
    init();
    
    while (renderer.step()) {
        object.updateUniformBuffer(renderer.getFrameIndex(), renderer.getSwapChainExtentWidth(), renderer.getSwapChainExtentHeight());
        renderer.drawFrame(object.descriptorSets, model.getNumIndices());
    }

    renderer.cleanup();
}