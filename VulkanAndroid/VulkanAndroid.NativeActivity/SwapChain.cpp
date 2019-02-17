#include "SwapChain.h"
#include "SurfaceSupportDetails.h"
#include "SwapChainImage.h"
#include <algorithm>

SwapChain::SwapChain(Device *device, VkSurfaceKHR surface, VkExtent2D surfaceExtent) : device(device), surface(surface)
{
	create(surfaceExtent);
	createImageViews();
}

SwapChain::~SwapChain()
{
	cleanup();
}

VkSwapchainKHR SwapChain::get() const
{
	return swapChain;
}

std::vector<VkImageView> SwapChain::getImageViews() const
{
	return imageViews;
}

VkExtent2D SwapChain::getExtent() const
{
	return extent;
}

uint32_t SwapChain::getImageCount() const
{
	return uint32_t(images.size());
}

VkFormat SwapChain::getImageFormat() const
{
    return imageFormat;
}

void SwapChain::recreate(VkExtent2D newExtent)
{
	cleanup();

	create(newExtent);

	createImageViews();
}

void SwapChain::recreate(VkSurfaceKHR surface, VkExtent2D newExtent)
{
    this->surface = surface;

    recreate(newExtent);
}

void SwapChain::create(VkExtent2D surfaceExtent)
{
    // get necessary swapchain properties
	const auto details = device->getSurfaceSupportDetails();
    const auto surfaceFormat = chooseSurfaceFormat(details.getFormats());
    const auto presentMode = choosePresentMode(details.getPresentModes());
	const auto surfaceCapabilities = details.getCapabilities();

	extent = chooseExtent(details.getCapabilities(), surfaceExtent);
	imageFormat = surfaceFormat.format;

	VkSwapchainCreateInfoKHR createInfo{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		surface,
        surfaceCapabilities.minImageCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		extent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		surfaceCapabilities.currentTransform,			
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,				
		presentMode,									
		true,
	    nullptr
	};

	// concurrent sharing mode only when using different queue families
    const QueueFamilyIndices queueFamilyIndices = device->getQueueFamilyIndices();
	std::vector<uint32_t> indices{
		queueFamilyIndices.getGraphics(),
		queueFamilyIndices.getPresentation()
	};
	if (indices[0] != indices[1])
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = uint32_t(indices.size());
		createInfo.pQueueFamilyIndices = indices.data();
	}

    CALL_VK(vkCreateSwapchainKHR(device->get(), &createInfo, nullptr, &swapChain));
    LOGI("SwapChain created.");

	saveImages();

    LOGD("SwapChain extent: %d x %d.", extent.width, extent.height);
    LOGD("SwapChain format: %d.", imageFormat);
    LOGD("SwapChain present mode: %d.", presentMode);
    LOGD("SwapChain image count: %d.", uint32_t(images.size()));
}

VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats) const
{
	// can choose any format
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return PREFERRED_PRESENT_FORMAT;
	}

	// try to found preferred format from available
	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == PREFERRED_PRESENT_FORMAT.format &&
			availableFormat.colorSpace == PREFERRED_PRESENT_FORMAT.colorSpace)
		{
			return availableFormat;
		}
	}

	// first available format
	return availableFormats[0];
}

VkPresentModeKHR SwapChain::choosePresentMode(std::vector<VkPresentModeKHR> availablePresentModes) const
{
	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == PREFERRED_PRESENT_MODE)
		{
			return availablePresentMode;
		}
	}

	// simplest mode
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseExtent(VkSurfaceCapabilitiesKHR capabilities, VkExtent2D actualExtent)
{
	// can choose any extent
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
	{
		return capabilities.currentExtent;
	}

    // extent height and width: (minAvailable <= extent <= maxAvailable) && (extent <= actualExtent)
    actualExtent.width = (std::max)(
        capabilities.minImageExtent.width,
        (std::min)(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = (std::max)(
        capabilities.minImageExtent.height,
        (std::min)(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

void SwapChain::saveImages()
{
    uint32_t imageCount;

	// real count of images can be greater than requested
	vkGetSwapchainImagesKHR(device->get(), swapChain, &imageCount, nullptr);  // get count
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device->get(), swapChain, &imageCount, images.data());  // get images
}

void SwapChain::createImageViews()
{
	imageViews.resize(getImageCount());

	for (uint32_t i = 0; i < getImageCount(); i++)
	{
		imageViews[i] = SwapChainImage(device, images[i], imageFormat).getView();
	}
}

void SwapChain::cleanup()
{
	for (auto imageView : imageViews)
    {
		vkDestroyImageView(device->get(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(device->get(), swapChain, nullptr);
}
