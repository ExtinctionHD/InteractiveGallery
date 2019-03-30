#include "Skybox.h"
#include "ActivityManager.h"

Skybox::Skybox(Device *device, const std::string &texturePath) : Model(device)
{
    std::vector<std::vector<uint8_t>> buffers(CUBE_MAP_FILES.size());
    for(uint32_t i = 0; i < buffers.size(); i++)
    {
        buffers[i] = ActivityManager::readAsset(texturePath + CUBE_MAP_FILES[i]);
    }

    cubeTexture = new TextureImage(device, buffers,  true, true);
    cubeTexture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
    cubeTexture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

Skybox::~Skybox()
{
    delete cubeTexture;
}

std::vector<DescriptorInfo> Skybox::getTextureInfos() const
{
    return { cubeTexture->getCombineSamplerInfo() };
}

std::vector<DescriptorInfo> Skybox::getUniformBufferInfos() const
{
    return {};
}
