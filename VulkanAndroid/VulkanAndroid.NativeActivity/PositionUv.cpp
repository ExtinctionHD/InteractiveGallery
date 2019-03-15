#include "PositionUv.h"

VkVertexInputBindingDescription PositionUv::getBindingDescription(uint32_t binding)
{
    return VkVertexInputBindingDescription{
        binding,
        sizeof(PositionUv),
        VK_VERTEX_INPUT_RATE_VERTEX
    };
}

std::vector<VkVertexInputAttributeDescription> PositionUv::getAttributeDescriptions(
    uint32_t binding,
    uint32_t locationOffset)
{
    const VkVertexInputAttributeDescription posDescription{
        locationOffset + 0,
        binding,
        VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(PositionUv, pos)
    };

    const VkVertexInputAttributeDescription uvDescription{
        locationOffset + 1,
        binding,
        VK_FORMAT_R32G32_SFLOAT,
        offsetof(PositionUv, uv)
    };

    return std::vector<VkVertexInputAttributeDescription>{ posDescription, uvDescription };
}
