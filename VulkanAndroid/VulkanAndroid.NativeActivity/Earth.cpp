#include "Earth.h"

Earth::Earth(Device *device, const std::string &texturePath) : Model(device), textures(EARTH_TEXTURE_TYPE_COUNT)
{
    for (size_t i = 0; i < textures.size(); i++)
    {
        textures[i] = new TextureImage(
            device,
            { texturePath + TEXTURE_FILES[i] },
            false);
        textures[i]->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
        textures[i]->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    }
}

Earth::~Earth()
{
    for(auto texture : textures)
    {
        delete texture;
    }
}

std::vector<TextureImage*> Earth::getTextures() const
{
    return textures;
}
