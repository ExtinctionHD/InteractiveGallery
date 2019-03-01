#pragma once
#include "Device.h"

class Pipeline
{
public:
    virtual ~Pipeline();

    VkPipeline get() const;

    VkPipelineLayout getLayout() const;

    void recreate();

protected:
    Pipeline(
        Device *device,
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        std::vector<VkPushConstantRange> pushConstantRanges);

    Device *device;

    VkPipeline pipeline;

    VkPipelineLayout layout;

    void createLayout(
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        std::vector<VkPushConstantRange> pushConstantRanges);

    virtual void createPipeline() = 0;
};

