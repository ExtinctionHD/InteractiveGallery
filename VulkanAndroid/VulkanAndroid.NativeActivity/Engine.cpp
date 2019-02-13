#include "Engine.h"
#include "Vertex.h"

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

    const std::string shadersPath = "shaders/Main/";
    const std::vector<std::shared_ptr<ShaderModule>> shaders{
        std::make_shared<ShaderModule>(device, shadersPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        std::make_shared<ShaderModule>(device, shadersPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    graphicsPipeline = new GraphicsPipeline(
        device,
        mainRenderPass,
        {},
        {},
        shaders,
        { Vertex::getBindingDescription(0) },
        Vertex::getAttributeDescriptions(0, 0),
        true);

    LOGI("Engine initialized.");
}

Engine::~Engine()
{
    delete swapChain;
	delete device;
	delete surface;
	delete instance;
}