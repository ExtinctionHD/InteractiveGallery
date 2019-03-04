#pragma once
#include "Device.h"
#include "SwapChainImage.h"

class SwapChain
{
public:
	SwapChain(Device *device, VkSurfaceKHR surface, VkExtent2D surfaceExtent);

	~SwapChain();

	VkSwapchainKHR get() const;

	std::vector<VkImageView> getImageViews() const;

	VkExtent2D getExtent() const;

	uint32_t getImageCount() const;

	VkFormat getImageFormat() const;

	void recreate(VkExtent2D newExtent);

    void recreate(VkSurfaceKHR surface, VkExtent2D newExtent);

private:
	const VkSurfaceFormatKHR PREFERRED_PRESENT_FORMAT = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	const VkPresentModeKHR PREFERRED_PRESENT_MODE = VK_PRESENT_MODE_MAILBOX_KHR;

	Device *device;

	VkSurfaceKHR surface;

	VkSwapchainKHR swapChain;

	VkExtent2D extent;

	VkFormat imageFormat;

    std::vector<SwapChainImage*> images;

	void create(VkExtent2D surfaceExtent);

	VkSurfaceFormatKHR chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats) const;

	VkPresentModeKHR choosePresentMode(std::vector<VkPresentModeKHR> availablePresentModes) const;

	static VkExtent2D chooseExtent(VkSurfaceCapabilitiesKHR capabilities, VkExtent2D actualExtent);

	void saveImages();

	void cleanup();
};

