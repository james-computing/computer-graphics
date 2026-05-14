#include "../include/application.hpp"

void Application::run() {
    initVulkan();
}

void Application::initVulkan() {
    //core
    core.initVulkan();



    // Model loading
    // Loads the texture and vertices.
    model.load();

    texture.init(core, model.textureWidth, model.textureHeight, model.pixels);
}

