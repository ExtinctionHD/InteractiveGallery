#pragma once
#include "Model.h"
#include "TextureImage.h"

class PhotoCard : public Model
{
public:
    PhotoCard(Device *device, const std::string &texturePath);

    virtual ~PhotoCard();

    TextureImage* getTexture() const;

    void setLocation(float latitude, float longitude);

private:
    const std::string TEXTURE_FILE = "Photo.jpg";

    TextureImage *texture;

    glm::vec3 position = glm::vec3(0.0f);

    glm::vec2 angle = glm::vec2(0.0f);
};