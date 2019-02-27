#pragma once
#include "Image.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "TextureImage.h"

class MainRenderPass : public RenderPass
{
public:
	MainRenderPass(Device *device, VkExtent2D attachmentExtent);

    uint32_t getColorAttachmentCount() const override;

    std::vector<VkClearValue> getClearValues() const override;

    TextureImage* getTexture() const;

protected:
    void createAttachments() override;

	void createRenderPass() override;

	void createFramebuffers() override;

private:
	std::shared_ptr<Image> colorImage;

	std::shared_ptr<Image> depthImage;

    std::shared_ptr<TextureImage> resolveTexture;
};

