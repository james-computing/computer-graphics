#pragma once

#include "core.hpp"
#include "renderer.hpp"
#include "descriptor.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "object.hpp"

class Application {
private:
    Core core;
    Renderer renderer;

    Camera camera;

    Model model;
    std::string const modelPath {"./models/viking_room.obj"};
    std::string const texturePath {"./textures/viking_room.png"};
    Object object;

public:
    void run();

private:
    void init(); 
};