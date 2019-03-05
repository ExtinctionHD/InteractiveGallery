#pragma once
#include "Pipeline.h"
#include <memory>
#include "ShaderModule.h"

class ComputePipeline : public Pipeline
{
public:
    ComputePipeline(
        Device *device,
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        std::vector<VkPushConstantRange> pushConstantRanges,
        std::shared_ptr<ShaderModule> shaderModule);

    ~ComputePipeline() = default;

private:
    std::shared_ptr<ShaderModule> shaderModule;

    void createPipeline() override;
};

