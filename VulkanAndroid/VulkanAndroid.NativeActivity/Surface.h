#pragma once
#include <android/native_activity.h>

class Surface
{
public:
	Surface(VkInstance instance, ANativeWindow *window);
	~Surface();

	VkSurfaceKHR get() const;

private:
	VkInstance instance;

	VkSurfaceKHR surface;

	void createSurface(ANativeWindow *window);
};
