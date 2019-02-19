#pragma once
#include "Device.h"
#include <glm/gtx/transform.hpp>
#include "Buffer.h"

class Camera
{
public:
    struct Attributes
    {
        VkExtent2D extent;
        glm::vec3 position;
        glm::vec3 target;
        glm::vec3 up;
        float fov;
        float nearPlane;
        float farPlane;
    };

    Camera(Device *device, Attributes attributes);

    ~Camera();

    glm::vec3 getPos() const;

    glm::vec3 getTarget() const;

    glm::vec3 getUp() const;

    Buffer* getBuffer() const;

    void update() const;

    void resize(VkExtent2D newExtent);

private:
    Attributes attributes;

    Buffer *buffer;

    glm::mat4 createViewMatrix() const;

    glm::mat4 createProjectionMatrix() const;
};

