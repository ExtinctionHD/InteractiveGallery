#include "Earth.h"

Earth::Earth(Device *device, const std::string &texturePath) : Model(device), textures(COUNT)
{
    for (size_t i = 0; i < textures.size(); i++)
    {
        textures[i] = new TextureImage(
            device,
            { texturePath + TEXTURE_FILES[i] },
            false,
            VK_FILTER_LINEAR,
            VK_SAMPLER_ADDRESS_MODE_REPEAT);
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
