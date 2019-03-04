#include "SwapChainImage.h"

SwapChainImage::SwapChainImage(Device *device, VkImage image, VkFormat format)
    : device(device), image(image), format(format)
{
	const VkImageSubresourceRange subresourceRange{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1
	};

	createView(subresourceRange, VK_IMAGE_VIEW_TYPE_2D);
}

SwapChainImage::~SwapChainImage()
{
    vkDestroyImageView(device->get(), view, nullptr);
}

VkImage SwapChainImage::get() const
{
	return image;
}

VkImageView SwapChainImage::getView() const
{
	return view;
}

VkFormat SwapChainImage::getFormat() const
{
	return format;
}

void SwapChainImage::createView(VkImageSubresourceRange subresourceRange, VkImageViewType viewType)
{
	VkImageViewCreateInfo createInfo{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
		0,
		image,
		viewType,
		format,
        VkComponentMapping{},
		subresourceRange,
	};

	CALL_VK(vkCreateImageView(device->get(), &createInfo, nullptr, &view));
}
