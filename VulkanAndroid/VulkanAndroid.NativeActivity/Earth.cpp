#include "Earth.h"
#include <glm/ext/matrix_transform.inl>
#include "utils.h"
#include "ActivityManager.h"
#include "sphere.h"

Earth::Earth(Device *device, const std::string &texturePath) : Model(device), textures(EARTH_TEXTURE_TYPE_COUNT)
{
    for (uint32_t i = 0; i < textures.size(); i++)
    {
        textures[i] = new TextureImage(
            device,
            { ActivityManager::readAsset(texturePath + TEXTURE_FILES[i]) },
            true,
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

std::vector<DescriptorInfo> Earth::getTextureInfos() const
{
    std::vector<DescriptorInfo> result;
    result.reserve(textures.size());

    for (const auto &texture : textures)
    {
        result.push_back(texture->getCombineSamplerInfo());
    }

    return result;
}

std::vector<DescriptorInfo> Earth::getUniformBufferInfos() const
{
    return {};
}

float Earth::getAngle() const
{
    return angle;
}

float Earth::getRadius() const
{
    return sphere::R;
}

void Earth::rotate(float angle)
{
    this->angle += angle;
    setTransformation(glm::rotate(getTransformation(), glm::radians(angle), -axis::Y));
}
