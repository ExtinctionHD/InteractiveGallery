#include "Camera.h"

Camera::Camera(Device *device, Parameters parameters, Location location) : parameters(parameters)
{
    glm::mat4 matrices[2] = { createViewMatrix(), createProjectionMatrix() };

    buffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof matrices);
    buffer->updateData(matrices);
}

Camera::~Camera()
{
    delete buffer;
}

glm::vec3 Camera::getPosition() const
{
    return location.position;
}

glm::vec3 Camera::getTarget() const
{
    return location.target;
}

glm::vec3 Camera::getUp() const
{
    return location.up;
}

glm::vec3 Camera::getRight() const
{
    const glm::vec3 forward = normalize(location.target - location.position);
    return cross(forward, location.up);
}

Buffer* Camera::getBuffer() const
{
    return buffer;
}

void Camera::update(Location location)
{
    this->location = location;

    glm::mat4 view = createViewMatrix();
    buffer->updateData(&view, 0, sizeof(glm::mat4));
}

void Camera::resize(VkExtent2D newExtent)
{
    parameters.extent = newExtent;
    glm::mat4 projection = createProjectionMatrix();
    buffer->updateData(&projection, sizeof(glm::mat4), sizeof(glm::mat4));
}

glm::mat4 Camera::createViewMatrix() const
{
    return lookAt(location.position, location.target, location.up);
}

glm::mat4 Camera::createProjectionMatrix() const
{
    const float aspect = parameters.extent.width / float(parameters.extent.height);

    const float fovY = aspect < 1.0f ? parameters.fov : parameters.fov / aspect;

    LOGD("Camera fov x: %f, y: %f", fovY * aspect, fovY);

    glm::mat4 projection = glm::perspective(
        glm::radians(fovY),
        aspect,
        parameters.nearPlane,
        parameters.farPlane);

    projection[1][1] *= -1;

    return projection;
}
