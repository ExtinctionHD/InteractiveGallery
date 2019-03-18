#pragma once
#include "Model.h"
#include "TextureImage.h"

enum EarthTextureType
{
    EARTH_TEXTURE_TYPE_DAY,
    EARTH_TEXTURE_TYPE_NIGHT,
    EARTH_TEXTURE_TYPE_NORMAL,
    EARTH_TEXTURE_TYPE_COUNT
};

class Earth :
    public Model
{
public:
    Earth(Device *device, const std::string &texturePath);

    virtual ~Earth();

    std::vector<TextureImage*> getTextures() const;

    float getAngle() const;

    float getRadius() const;

    void rotate(float angle);

private:
    const std::vector<std::string> TEXTURE_FILES{
        "Day.jpg", "Night.jpg", "Normal.jpg"
    };

    std::vector<TextureImage*> textures;

    float angle{};
};

