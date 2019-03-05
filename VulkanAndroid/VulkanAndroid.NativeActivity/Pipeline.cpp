#include "Pipeline.h"

Pipeline::~Pipeline()
{
    vkDestroyPipeline(device->get(), pipeline, nullptr);
    vkDestroyPipelineLayout(device->get(), layout, nullptr);
}

VkPipeline Pipeline::get() const
{
    return pipeline;
}

VkPipelineLayout Pipeline::getLayout() const
{
    return layout;
}

void Pipeline::recreate()
{
    vkDestroyPipeline(device->get(), pipeline, nullptr);
    createPipeline();
}

Pipeline::Pipeline(
    Device *device,
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
    std::vector<VkPushConstantRange> pushConstantRanges)
    : device(device)
{
    createLayout(descriptorSetLayouts, pushConstantRanges);
}

void Pipeline::createLayout(
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
    std::vector<VkPushConstantRange> pushConstantRanges)
{
    VkPipelineLayoutCreateInfo createInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        uint32_t(descriptorSetLayouts.size()),
        descriptorSetLayouts.data(),
        uint32_t(pushConstantRanges.size()),
        pushConstantRanges.data(),
    };

    CALL_VK(vkCreatePipelineLayout(device->get(), &createInfo, nullptr, &layout));
}
