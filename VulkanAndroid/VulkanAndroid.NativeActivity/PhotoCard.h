#pragma once
#include "Model.h"
#include "TextureImage.h"
#include "Earth.h"
#include <glm/common.hpp>

class PhotoCard : public Model
{
public:
    PhotoCard(Device *device, const std::string &texturePath, Earth *earth);

    virtual ~PhotoCard();

    TextureImage* getTexture() const;

    void update(float latitude, float longitude, glm::vec3 cameraPos, glm::vec3 cameraRight);

private:
    const std::string TEXTURE_FILE = "Photo.jpg";

    Earth *earth;

    TextureImage *texture;

    glm::vec3 position = glm::vec3(0.0f);
};
