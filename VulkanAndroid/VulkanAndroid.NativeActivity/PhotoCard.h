#pragma once
#include "Model.h"
#include "TextureImage.h"
#include "Earth.h"
#include "Camera.h"
#include "Controller.h"

class PhotoCard : public Model
{
public:
    PhotoCard(Device *device, const std::string &texturePath, Earth *earth, Camera *camera, Controller *controller);

    virtual ~PhotoCard();

    TextureImage* getTexture() const;

    Buffer* getOpacityBuffer() const;

    void update();

private:
    const std::string TEXTURE_FILE = "Photo.jpg";

    const float MAX_DISTANCE = 20.0f;

    const glm::vec2 COORDINATES = glm::vec2(2.293966f, 48.858187f);

    Buffer *opacityBuffer;

    Earth *earth;

    Camera *camera;

    Controller *controller;

    TextureImage *texture;

    glm::vec3 position = glm::vec3(0.0f);

    float loopDistance(glm::vec2 a, glm::vec2 b);
};
