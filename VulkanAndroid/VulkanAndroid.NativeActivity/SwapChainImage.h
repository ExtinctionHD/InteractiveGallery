#pragma once
#include "Device.h"

class SwapChainImage
{
public:
	virtual ~SwapChainImage() = default;

	SwapChainImage(Device *device, VkImage image, VkFormat format);

	VkImage get() const;

	VkImageView getView() const;

	VkFormat getFormat() const;

	VkImageView createImageView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType) const;

protected:
    SwapChainImage() = default;

	Device *device;

	VkImage image;

	VkImageView view;

	VkFormat format;
};

