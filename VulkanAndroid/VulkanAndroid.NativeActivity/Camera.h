#pragma once
#include "Device.h"
#include <glm/gtx/transform.hpp>
#include "Buffer.h"

class Camera
{
public:
    struct Parameters
    {
        VkExtent2D extent;
        float fov;
        float nearPlane;
        float farPlane;
    };

    struct Location
    {
        glm::vec3 position;
        glm::vec3 target;
        glm::vec3 up;
    };

    Camera(Device *device, Parameters parameters, Location location);

    ~Camera();

    glm::vec3 getPosition() const;

    glm::vec3 getTarget() const;

    glm::vec3 getUp() const;

    glm::vec3 getRight() const;

    Buffer* getBuffer() const;

    void update(Location location);

    void resize(VkExtent2D newExtent);

private:
    Parameters parameters;

    Location location;

    Buffer *buffer;

    glm::mat4 createViewMatrix() const;

    glm::mat4 createProjectionMatrix() const;
};

