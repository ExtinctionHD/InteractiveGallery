#pragma once
#include "Image.h"
#include "SwapChain.h"
#include "RenderPass.h"

class MainRenderPass : public RenderPass
{
public:
	MainRenderPass(Device *device, SwapChain *swapChain);

    uint32_t getColorAttachmentCount() const override;

protected:
    void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
    SwapChain *swapChain;

	std::shared_ptr<Image> colorImage;

	std::shared_ptr<Image> depthImage;
};

