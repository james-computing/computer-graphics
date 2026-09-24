#pragma once

#include "core.hpp"
#include "renderer.hpp"
#include "descriptor.hpp"
#include "camera.hpp"
#include "modelsInstances.hpp"

class Application {
private:
    Core core;
    Renderer renderer;

    Camera camera;

    std::vector<std::string_view> const modelPaths {"./models/viking_room.obj", "./models/box_01.obj"};
    std::vector<std::string_view> const texturePaths {"./textures/viking_room.png", "./textures/tex_box_01_d.jpg"};

public:
    void run();

private:
    void init(); 
};