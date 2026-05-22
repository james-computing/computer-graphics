#pragma once

#include "core.hpp"
#include "renderer.hpp"
#include "model.hpp"
#include "object.hpp"
#include "textureSampler.hpp"

class Application {
private:
    Core core;
    Renderer renderer;

    Model model;
    std::string const modelPath {"./models/viking_room.obj"};
    std::string const texturePath {"./textures/viking_room.png"};
    Object object;

    vk::raii::Sampler textureSampler {nullptr};

public:
    void run();

private:
    void init(); 
};