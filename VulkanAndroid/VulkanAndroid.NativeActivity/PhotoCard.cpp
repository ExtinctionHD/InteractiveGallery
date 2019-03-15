#include "PhotoCard.h"

PhotoCard::PhotoCard(Device *device, const std::string &texturePath) : Model(device)
{
    texture = new TextureImage(
        device,
        { texturePath + TEXTURE_FILE },
        false);
    texture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
    texture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
}

PhotoCard::~PhotoCard()
{
    delete texture;
}

TextureImage * PhotoCard::getTexture() const
{
    return texture;
}
