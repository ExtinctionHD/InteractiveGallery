#include "Skybox.h"

Skybox::Skybox(Device *device, const std::string &texturePath) : Model(device)
{
    std::vector<std::string> paths(CUBE_MAP_FILES.size());
    for(size_t i = 0; i < paths.size(); i++)
    {
        paths[i] = texturePath + CUBE_MAP_FILES[i];
    }

    cubeTexture = new TextureImage(device, paths, true);
    cubeTexture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
    cubeTexture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

Skybox::~Skybox()
{
    delete cubeTexture;
}

TextureImage* Skybox::getCubeTexture() const
{
    return cubeTexture;
}
