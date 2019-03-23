#pragma once
#include "PositionUv.h"

namespace card
{
    const float WIDTH = 2.0f;
    const float HEIGHT = 2.15f;

    const std::vector<PositionUv> VERTICES{
        { glm::vec3(-1.0f, -2.15f, 0.0f), glm::vec2(1.02f, -0.02f) },
        { glm::vec3(1.0f, -2.15f, 0.0f), glm::vec2(-0.02f,-0.02f) },
        { glm::vec3(1.0f, -0.15f, 0.0f), glm::vec2(-0.02f, 1.02f) },
        { glm::vec3(-1.0f, -0.15f, 0.0f), glm::vec2(1.02f, 1.02f) },

        { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.02, 1.02f) },
        { glm::vec3(-0.05f, -0.15f, 0.0f), glm::vec2(1.02, 1.02f) },
        { glm::vec3(0.05f, -0.15f, 0.0f), glm::vec2(1.02, 1.02f) },
    };

    const std::vector<uint32_t> INDICES{
        0, 1, 3,
        1, 2, 3,
        4, 5, 6,
    };

}
