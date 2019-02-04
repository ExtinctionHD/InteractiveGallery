#pragma once
#define VK_USE_PLATFORM_ANDROID_KHR
#include "vulkan_wrapper.h"
#include "Instance.h"

class Core
{
public:
	Core();
	~Core();

private:
	Instance *instance;
};

