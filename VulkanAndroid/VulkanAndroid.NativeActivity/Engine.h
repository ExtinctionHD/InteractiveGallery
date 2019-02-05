#pragma once
#define VK_USE_PLATFORM_ANDROID_KHR
#include "vulkan_wrapper.h"
#include "Instance.h"

class Engine
{
public:
	Engine();
	~Engine();

private:
	Instance *instance;
};

