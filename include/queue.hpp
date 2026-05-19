#pragma once

struct Queue {
    uint32_t index;
    vk::raii::Queue vkraii {nullptr};
};