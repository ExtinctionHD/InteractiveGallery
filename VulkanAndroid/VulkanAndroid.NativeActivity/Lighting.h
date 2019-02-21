#pragma once
#include "Device.h"
#include "Buffer.h"

class Lighting
{
public:
    struct Attributes
    {
        glm::vec3 color;
        float ambientStrength;
        glm::vec3 direction;
        float directedStrength;
        glm::vec3 cameraPos;
        float specularPower;
    };

    Lighting(Device *device, Attributes attributes);

    ~Lighting();

    Buffer* getBuffer() const;

    void update(glm::vec3 cameraPos);

private:
    Attributes attributes;

    Buffer *buffer;
};

