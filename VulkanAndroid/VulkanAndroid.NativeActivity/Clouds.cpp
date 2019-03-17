#include "Clouds.h"
#include <glm/gtx/transform.hpp>
#include "ActivityManager.h"

Clouds::Clouds(Device *device, const std::string &texturePath) : Model(device)
{
    texture = new TextureImage(
        device,
        { ActivityManager::readAsset(texturePath + TEXTURE_FILE) },
        true,
        false);
    texture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
    texture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

Clouds::~Clouds()
{
    delete texture;
}

TextureImage * Clouds::getTexture() const
{
    return texture;
}

void Clouds::setEarthTransformation(glm::mat4 earthTransformation)
{
    setTransformation(scale(earthTransformation, glm::vec3(RADIUS_SCALE)));
}
