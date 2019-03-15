#pragma once

struct PositionUv
{
    glm::vec3 pos = glm::vec3(0.0f);

    glm::vec2 uv = glm::vec2(0.0f);

    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding);

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, uint32_t locationOffset);
};

