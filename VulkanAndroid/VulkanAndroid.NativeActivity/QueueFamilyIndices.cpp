#include "QueueFamilyIndices.h"

QueueFamilyIndices::QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);  // get count

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());  // get queue family properties

	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics = i;
		}

		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
		if (queueFamilies[i].queueCount > 0 && presentationSupport)
		{
            presentation = i;
		}

		if (completed())
		{
			break;
		}
	}
}

uint32_t QueueFamilyIndices::getGraphics() const
{
    LOGA(graphics >= 0, "No queue family suitable for drawing graphics.");

	return uint32_t(graphics);
}

uint32_t QueueFamilyIndices::getPresentation() const
{
    LOGA(presentation >= 0, "No queue family suitable for presentation.");

	return uint32_t(presentation);
}

bool QueueFamilyIndices::completed() const
{
	return graphics >= 0 && presentation >= 0;
}
