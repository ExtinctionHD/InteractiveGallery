#pragma once

class QueueFamilyIndices
{
public:
    // try to find required queue families and save they indices
	QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);

	uint32_t getGraphics() const;

    uint32_t getCompute() const;

	uint32_t getPresentation() const;

	// this device have all required queue families (for this surface)
	bool completed() const;

private:
	// queue family indices
	int graphics = -1;
    int compute = -1;
	int presentation = -1;
};

