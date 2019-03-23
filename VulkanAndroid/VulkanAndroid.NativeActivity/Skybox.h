#pragma once
#include "Model.h"
#include "TextureImage.h"

class Skybox : public Model
{
public:
    Skybox(Device *device, const std::string &texturePath);

    virtual ~Skybox();

    std::vector<DescriptorInfo> getTextureInfos() const override;

    std::vector<DescriptorInfo> getUniformBufferInfos() const override;

private:
    const std::vector<std::string> CUBE_MAP_FILES{
        "Back.png",
        "Down.png",
        "Front.png",
        "Left.png",
        "Right.png",
        "Top.png"
    };

    TextureImage *cubeTexture;
};

