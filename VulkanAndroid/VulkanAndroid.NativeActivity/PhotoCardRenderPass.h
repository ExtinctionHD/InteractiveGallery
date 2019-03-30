#pragma once
#include "RenderPass.h"
#include "SwapChain.h"

class GalleryRenderPass : public RenderPass
{
public:
    GalleryRenderPass(Device *device, SwapChain *swapChain, VkSampleCountFlagBits sampleCount);

    uint32_t getColorAttachmentCount() const override;

    std::vector<VkClearValue> getClearValues() const override;

protected:
    void createAttachments() override;

    void createRenderPass() override;

    void createFramebuffers() override;

private:
    SwapChain *swapChain;
};

