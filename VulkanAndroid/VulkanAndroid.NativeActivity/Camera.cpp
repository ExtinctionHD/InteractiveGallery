#include "Camera.h"

Camera::Camera(Device *device, Attributes attributes) : attributes(attributes)
{
    glm::mat4 matrices[2] = { createViewMatrix(), createProjectionMatrix() };

    buffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof matrices);
    buffer->updateData(matrices);
}

Camera::~Camera()
{
    delete buffer;
}

glm::vec3 Camera::getPos() const
{
    return attributes.position;
}

glm::vec3 Camera::getTarget() const
{
    return attributes.target;
}

glm::vec3 Camera::getUp() const
{
    return  attributes.up;
}

Buffer * Camera::getBuffer() const
{
    return buffer;
}

void Camera::update() const
{
    glm::mat4 view = createProjectionMatrix();
    buffer->updateData(&view, 0, sizeof(glm::mat4));
}

void Camera::resize(VkExtent2D newExtent)
{
    attributes.extent = newExtent;
    glm::mat4 projection = createProjectionMatrix();
    buffer->updateData(&projection, sizeof(glm::mat4), sizeof(glm::mat4));
}

glm::mat4 Camera::createViewMatrix() const
{
    return lookAt(attributes.position, attributes.target, attributes.up);
}

glm::mat4 Camera::createProjectionMatrix() const
{
    const float aspect = attributes.extent.width / float(attributes.extent.height);

    const float fovY = aspect < 1.0f ? attributes.fov : attributes.fov / aspect;

    LOGD("Camera fov x: %f, y: %f", fovY * aspect, fovY);

    glm::mat4 projection = glm::perspective(
        glm::radians(fovY),
        aspect,
        attributes.nearPlane,
        attributes.farPlane);

    projection[1][1] *= -1;

    return projection;
}
