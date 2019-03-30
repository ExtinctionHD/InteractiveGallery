#include "Gallery.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "sphere.h"
#include "utils.h"
#include "ActivityManager.h"

Gallery::Gallery(
    Device *device,
    const std::string &path,
    Earth *earth,
    Camera *camera,
    Controller *controller)
    : Model(device),
    earth(earth),
    camera(camera),
    controller(controller)
{
    loadPhotographs(device, path);

    opacityBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float));
}

Gallery::~Gallery()
{
    delete opacityBuffer;

    for (auto texture : textures)
    {
        delete texture;
    }
}

std::vector<TextureImage*> Gallery::getTextures() const
{
    return textures;
}

Buffer* Gallery::getOpacityBuffer() const
{
    return opacityBuffer;
}

void Gallery::update()
{
    const glm::vec2 cameraCoordinates = controller->getCoordinates(earth->getAngle());

    size_t index;
    float nearestDistance = 360.0f;
    for (size_t i = 0; i < COORDINATES.size(); i++)
    {
        const float distance = loopDistance(cameraCoordinates, COORDINATES[i]);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            index = i;
        }
    }

    float opacity = 0.0f;

    if (nearestDistance < MAX_DISTANCE)
    {
        opacity = calculateOpacity(nearestDistance);

        setLocation(COORDINATES[index]);
    }

    opacityBuffer->updateData(&opacity);
}

void Gallery::loadPhotographs(Device *device, const std::string &path)
{
    std::vector<std::string> fileNames = ActivityManager::getFileNames(path, EXTENSIONS);

    textures.reserve(fileNames.size());

    for (const auto &fileName : fileNames)
    {
        TextureImage *texture = new TextureImage(
            device,
            { ActivityManager::read(fileName) },
            false);
        texture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
        texture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
        textures.push_back(texture);
    }
}

float Gallery::loopDistance(glm::vec2 a, glm::vec2 b)
{
    float result;

    if (glm::abs(a.x - b.x) < 180.0f)
    {
        result = distance(a, b);
    }
    else
    {
        if (a.x < b.x)
        {
            a.x += 360.0f;
        }
        else
        {
            b.x += 360.0f;
        }
        result = distance(a, b);
    }

    return result;
}

float Gallery::calculateOpacity(float nearestDistance)
{
    const float minDistance = MAX_DISTANCE / 2.0f;

    float opacity = 1.0f - (nearestDistance - minDistance) / (MAX_DISTANCE - minDistance);
    opacity = opacity > 1.0f ? 1.0f : opacity;
    opacity = std::pow(opacity, 0.5f);

    return opacity;
}

void Gallery::setLocation(glm::vec2 photoCoordinates)
{
    position = sphere::R * axis::rotate(
        -axis::X,
        glm::vec2(photoCoordinates.x + earth->getAngle(), photoCoordinates.y),
        nullptr);

    const glm::vec3 direction = normalize(camera->getPosition() - position);
    const glm::vec2 angle(glm::radians(90.0f) + std::atan2(direction.z, direction.x), glm::asin(direction.y));

    glm::mat4 transformation = translate(glm::mat4(1.0f), position);
    transformation = rotate(transformation, angle.y, camera->getRight());
    transformation = rotate(transformation, angle.x, -axis::Y);
    transformation = scale(transformation, glm::vec3(5.0f, 2.8f, 1.0f));

    setTransformation(transformation);
}
