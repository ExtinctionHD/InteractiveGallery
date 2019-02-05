#include "Surface.h"

Surface::Surface(VkInstance instance, ANativeWindow *window) : instance(instance)
{
	createSurface(window);
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

VkSurfaceKHR Surface::get() const
{
	return surface;
}

void Surface::createSurface(ANativeWindow *window)
{
	VkAndroidSurfaceCreateInfoKHR createInfo{
		VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		window
	};

	CALL_VK(vkCreateAndroidSurfaceKHR(instance, &createInfo, nullptr, &surface));
	LOGI("Android surface created");
}
