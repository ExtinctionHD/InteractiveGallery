#pragma once
#include "Device.h"
#include "Buffer.h"

class Lighting
{
public:
    struct Attributes
    {
        glm::vec3 direction;
        float directedIntensity;
        glm::vec3 cameraPos;
        float transitionFactor;
        float ambientIntensity;
    };

    Lighting(Device *device, Attributes attributes);

    ~Lighting();

    Buffer* getBuffer() const;

    void update(glm::vec3 cameraPos);

private:
    Attributes attributes;

    Buffer *buffer;
};

