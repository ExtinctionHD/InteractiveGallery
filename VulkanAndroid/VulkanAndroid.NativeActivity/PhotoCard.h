#pragma once
#include "Model.h"
#include "TextureImage.h"
#include "Earth.h"

class PhotoCard : public Model
{
public:
    PhotoCard(Device *device, const std::string &texturePath, Earth *earth);

    virtual ~PhotoCard();

    TextureImage* getTexture() const;

    void setLocation(float latitude, float longitude);

private:
    const std::string TEXTURE_FILE = "Photo.jpg";

    Earth *earth;

    TextureImage *texture;

    glm::vec3 position = glm::vec3(0.0f);
};