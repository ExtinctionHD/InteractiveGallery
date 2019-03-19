#pragma once

namespace math
{
    uint32_t ceilLog2(uint32_t x);
}

namespace window
{
    VkExtent2D getExtent(ANativeWindow *window);
}

namespace axis
{
    const glm::vec3 X = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 Y = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 Z = glm::vec3(0.0f, 0.0f, 1.0f);

    // rotates baseAxis around -Y on angle.x and then around horizontal axis on angle.y
    glm::vec3 rotate(glm::vec3 baseAxis, glm::vec2 angle, glm::vec3 *outHorizontalAxis);
}

namespace file
{
    std::string getFileName(const std::string &path);
}