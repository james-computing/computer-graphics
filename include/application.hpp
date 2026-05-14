#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib> // For uint32_t
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::clamp
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/gtc/matrix_transform.hpp> // for model view projection
#include <unordered_map>

#include "../include/core.hpp"
#include "../include/model.hpp"
#include "../include/texture.hpp"

class Application {
public:
    void run();
private:
    ///////////////////////////////////////////////// MEMBER VARIABLES //////////////////////////////////
    Core core;

    Model model;
    Texture texture;

    /////////////////////////////////////// METHODS //////////////////////////////////////////////////

    void initVulkan();
};