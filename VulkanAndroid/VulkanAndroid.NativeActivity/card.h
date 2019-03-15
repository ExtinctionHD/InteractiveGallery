#pragma once
#include "PositionUv.h"

namespace card
{
    const float A = 2.0f;

    const std::vector<PositionUv> VERTICES{
        { glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
        { glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f) },
        { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
        { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
    };

    const std::vector<uint32_t> INDICES{
        0, 1, 3,
        1, 2, 3,
    };

}
