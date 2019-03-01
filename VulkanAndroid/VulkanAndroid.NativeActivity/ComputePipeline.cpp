#include "ComputePipeline.h"
#include <utility>

ComputePipeline::ComputePipeline(
    Device *device,
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
    std::vector<VkPushConstantRange> pushConstantRanges,
    std::shared_ptr<ShaderModule> shaderModule)
    : Pipeline(device, descriptorSetLayouts, pushConstantRanges),
      shaderModule(std::move(shaderModule))
{
    ComputePipeline::createPipeline();
}

void ComputePipeline::createPipeline()
{
    const VkPipelineShaderStageCreateInfo shaderStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        shaderModule->getStage(),
        shaderModule->getModule(),
        "main",
        shaderModule->getSpecializationInfo()
    };

    VkComputePipelineCreateInfo createInfo{
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        shaderStage,
        layout,
        nullptr,
        -1
    };

    CALL_VK(vkCreateComputePipelines(device->get(), nullptr, 1, &createInfo, nullptr, &pipeline));
}
