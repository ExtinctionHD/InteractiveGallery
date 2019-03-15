#pragma once
#include "Model.h"
#include "TextureImage.h"

class PhotoCard : public Model
{
public:
    PhotoCard(Device *device, const std::string &texturePath);

    virtual ~PhotoCard();

    TextureImage* getTexture() const;

private:
    const std::string TEXTURE_FILE = "Photo.jpg";

    TextureImage *texture;
};