#include "Gallery.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
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

    parameterBuffer = new Buffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Parameters));
}

Gallery::~Gallery()
{
    delete parameterBuffer;
    delete texture;
}

TextureImage* Gallery::getTexture() const
{
    return texture;
}

Buffer* Gallery::getParameterBuffer() const
{
    return parameterBuffer;
}

void Gallery::update()
{
    const glm::vec2 cameraCoordinates = controller->getCoordinates(earth->getAngle());

    uint32_t index = 0;
    float nearestDistance = 360.0f;

    for (uint32_t i = 0; i < COORDINATES.size(); i++)
    {
        const float distance = loopDistance(cameraCoordinates, COORDINATES[i]);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            index = i;
        }
    }

    Parameters parameters{ float(index), 0.0f };
    const float maxDistance = (controller->getRadius() - earth->getRadius()) * MAX_DISTANCE_FACTOR;

    if (nearestDistance < maxDistance)
    {
        parameters.opacity = calculateOpacity(nearestDistance, maxDistance);
        setTransformation(calculateTransformation(COORDINATES[index]));
    }

    parameterBuffer->updateData(&parameters);
}

void Gallery::loadPhotographs(Device *device, const std::string &path)
{
    std::vector<std::string> fileNames = ActivityManager::getFileNames(path, { ".jpg", ".jpeg", ".png" });

    std::vector<std::vector<uint8_t>> buffers;
    for (const auto &fileName : fileNames)
    {
        buffers.push_back(ActivityManager::read(fileName));
    }

    texture = new TextureImage( device, buffers, false, false);
    texture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
    texture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
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

float Gallery::calculateOpacity(float nearestDistance, float maxDistance)
{
    const float transparencyDistance = maxDistance / 2.0f;

    float opacity = 1.0f - (nearestDistance - transparencyDistance) / (maxDistance - transparencyDistance);

    opacity = opacity > 1.0f ? 1.0f : opacity;
    opacity = std::pow(opacity, 0.5f);

    return opacity;
}

glm::vec3 Gallery::calculateScale()
{
    const float distance = controller->getRadius() - earth->getRadius();

    const float biggerSide = distance * SCALE_FACTOR;

    const VkExtent3D extent = texture->getExtent();
    const float aspect = extent.width / float(extent.height);

    glm::vec3 scale;
    if (aspect > 1.0f)
    {
        scale = glm::vec3(biggerSide, biggerSide / aspect, 1.0f);
    }
    else
    {
        scale = glm::vec3(biggerSide / aspect, biggerSide, 1.0f);
    }

    return scale;
}

glm::mat4 Gallery::calculateTransformation(glm::vec2 photoCoordinates)
{
    const glm::vec3 position = earth->getRadius() * axis::rotate(
        -axis::X,
        glm::vec2(photoCoordinates.x + earth->getAngle(), photoCoordinates.y),
        nullptr);

    // TODO: fix y angle calculation 

    const glm::vec3 direction = normalize(camera->getPosition() - position);
    const glm::vec2 angle(glm::radians(90.0f) + std::atan2(direction.z, direction.x), glm::asin(direction.y));

    glm::mat4 transformation = translate(glm::mat4(1.0f), position);
    transformation = rotate(transformation, angle.y, camera->getRight());
    transformation = rotate(transformation, angle.x, -axis::Y);
    transformation = scale(transformation, calculateScale());

    return transformation;
}
