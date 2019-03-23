#pragma once
#include "Model.h"
#include "TextureImage.h"

class Clouds : public Model
{
public:
    Clouds(Device *device, const std::string &texturePath);

    virtual ~Clouds();

    std::vector<DescriptorInfo> getTextureInfos() const override;

    std::vector<DescriptorInfo> getUniformBufferInfos() const override;

    void setEarthTransformation(glm::mat4 earthTransformation);

private:
    const std::string TEXTURE_FILE = "Clouds.jpg";

    const float RADIUS_SCALE = 1.01f;

    TextureImage *texture;
};

