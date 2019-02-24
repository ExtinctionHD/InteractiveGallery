#pragma once

namespace math
{
    uint32_t ceilLog2(uint32_t x);
}

namespace window
{
    VkExtent2D getExtent(ANativeWindow *window);
}

namespace vector
{
    const glm::vec3 X = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 Y = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 Z = glm::vec3(0.0f, 0.0f, 1.0f);
}