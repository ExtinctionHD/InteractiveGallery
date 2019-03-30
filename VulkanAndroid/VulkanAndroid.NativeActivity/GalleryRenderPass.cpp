#include "GalleryRenderPass.h"

GalleryRenderPass::GalleryRenderPass(Device *device, SwapChain *swapChain, VkSampleCountFlagBits sampleCount)
    : RenderPass(device, swapChain->getExtent(), sampleCount),
      swapChain(swapChain)
{
}

uint32_t GalleryRenderPass::getColorAttachmentCount() const
{
    return 1;
}

std::vector<VkClearValue> GalleryRenderPass::getClearValues() const
{
    VkClearValue colorClearValue;
    colorClearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    return { colorClearValue };
}

void GalleryRenderPass::createAttachments()
{
}

void GalleryRenderPass::createRenderPass()
{
    // description of attachments

    const VkAttachmentDescription colorAttachmentDesc{
        0,
        swapChain->getImageFormat(),
        sampleCount,
        VK_ATTACHMENT_LOAD_OP_LOAD,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    std::vector<VkAttachmentDescription> attachmentDescriptions{
        colorAttachmentDesc,
    };

    // references to attachments

    VkAttachmentReference colorAttachmentRef{
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // subpass and it dependencies (contain references)

    VkSubpassDescription subpass{
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &colorAttachmentRef,
        nullptr,
        nullptr,
        0,
        nullptr
    };

    const VkSubpassDependency inputDependency{
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
    };

    const VkSubpassDependency outputDependency{
        0,
        VK_SUBPASS_EXTERNAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_DEPENDENCY_BY_REGION_BIT,
    };

    std::vector<VkSubpassDependency> dependencies{
        inputDependency,
        outputDependency
    };

    // render pass (contain descriptions)

    VkRenderPassCreateInfo createInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        uint32_t(attachmentDescriptions.size()),
        attachmentDescriptions.data(),
        1,
        &subpass,
        uint32_t(dependencies.size()),
        dependencies.data(),
    };

    CALL_VK(vkCreateRenderPass(device->get(), &createInfo, nullptr, &renderPass));
    LOGI("Card render pass created.");
}

void GalleryRenderPass::createFramebuffers()
{
    for (const auto image : swapChain->getImages())
    {
        addFramebuffer({ image->getView() });
    }
}
