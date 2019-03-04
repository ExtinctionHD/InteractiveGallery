#pragma once
#include "Device.h"

class SwapChainImage
{
public:
    SwapChainImage(Device *device, VkImage image, VkFormat format);

	virtual ~SwapChainImage();

	VkImage get() const;

	VkImageView getView() const;

	VkFormat getFormat() const;

protected:
    SwapChainImage() = default;

	Device *device;

	VkImage image;

	VkImageView view;

	VkFormat format;

    void createView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType);
};

