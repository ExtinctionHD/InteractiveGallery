#pragma once
#include "Model.h"
#include "TextureImage.h"
#include "Earth.h"
#include "Camera.h"
#include "Controller.h"

class Gallery : public Model
{
public:
    Gallery(Device *device, const std::string &path, Earth *earth, Camera *camera, Controller *controller);

    virtual ~Gallery();

    std::vector<TextureImage *> getTextures() const;

    Buffer* getOpacityBuffer() const;

    void update();

private:
    const float MAX_DISTANCE = 20.0f;

    const std::vector<std::string> EXTENSIONS{
        ".jpg", ".jpeg", ".png"
    };

    const std::vector<glm::vec2> COORDINATES{
        glm::vec2(2.293966f, 48.858187f),
        glm::vec2(151.215297, -33.856829f),
    };

    Buffer *opacityBuffer;

    Earth *earth;

    Camera *camera;

    Controller *controller;

    std::vector<TextureImage*> textures;

    glm::vec3 position = glm::vec3(0.0f);

    void loadPhotographs(Device *device, const std::string &path);

    float loopDistance(glm::vec2 a, glm::vec2 b);

    float calculateOpacity(float nearestDistance);

    void setLocation(glm::vec2 photoCoordinates);
};
