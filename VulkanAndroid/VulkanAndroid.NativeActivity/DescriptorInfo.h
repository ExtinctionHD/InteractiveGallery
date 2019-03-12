#pragma once

union DescriptorInfo
{
    VkDescriptorImageInfo image;
    VkDescriptorBufferInfo buffer;
};