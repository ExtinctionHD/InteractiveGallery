#include "Engine.h"

Engine::Engine(ANativeWindow *window)
{
	InitVulkan();

    const VkExtent2D extent{
        uint32_t(ANativeWindow_getWidth(window)),
        uint32_t(ANativeWindow_getHeight(window))
    };

    LOGD("Window extent: %d x %d.", extent.width, extent.height);

	instance = new Instance();
	surface = new Surface(instance->get(), window);
	device = new Device(instance->get(), surface->get(), instance->getLayers());
    swapChain = new SwapChain(device, surface->get(), extent);
    descriptorPool = new DescriptorPool(device, 1, 1, 1);

    mainRenderPass = new MainRenderPass(device, swapChain);
    mainRenderPass->create();

    LOGI("Engine initialized.");
}

Engine::~Engine()
{
    delete swapChain;
	delete device;
	delete surface;
	delete instance;
}