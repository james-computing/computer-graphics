#pragma once

struct Queue {
    uint32_t familyIndex;
    vk::raii::Queue vkraii {nullptr};
};