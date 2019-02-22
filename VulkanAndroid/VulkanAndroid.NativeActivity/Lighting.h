#pragma once
#include "Device.h"
#include "Buffer.h"

class Lighting
{
public:
    struct Attributes
    {
        glm::vec3 direction;
        float transitionFactor;
        glm::vec3 cameraPos;
    };

    Lighting(Device *device, Attributes attributes);

    ~Lighting();

    Buffer* getBuffer() const;

    void update(glm::vec3 cameraPos);

private:
    Attributes attributes;

    Buffer *buffer;
};

