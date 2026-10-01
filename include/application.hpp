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

public:
    void run();

private:
    void init(); 
};