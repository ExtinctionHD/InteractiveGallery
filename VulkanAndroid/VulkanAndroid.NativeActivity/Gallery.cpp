#include "Gallery.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <sstream>
#include "ActivityManager.h"
#include "cities.h"
#include <cstdlib>
#include <regex>

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

std::vector<DescriptorInfo> Gallery::getTextureInfos() const
{
    return { texture->getCombineSamplerInfo() };
}

std::vector<DescriptorInfo> Gallery::getUniformBufferInfos() const
{
    return { parameterBuffer->getUniformBufferInfo() };
}

void Gallery::update()
{
    if (!activated || empty)
    {
        return;
    }

    const glm::vec2 cameraCoordinates = controller->getCoordinates(earth->getAngle());

    uint32_t index;
    const float nearestDistance = calculateNearestDistance(cameraCoordinates, &index);

    Parameters parameters{ float(index), 0.0f };
    const float distanceLimit = (controller->getRadius() - earth->getRadius()) * DISTANCE_LIMIT_FACTOR;

    if (nearestDistance < distanceLimit)
    {
        parameters.opacity = calculateOpacity(nearestDistance, distanceLimit);
        setTransformation(calculateTransformation(coordinates[index], cameraCoordinates));
    }

    parameterBuffer->updateData(&parameters);
}

void Gallery::activate()
{
    activated = true;
}

void Gallery::loadPhotographs(Device *device, const std::string &path)
{
    std::vector<std::string> paths = ActivityManager::getFilePaths(path, { ".jpg", ".jpeg", ".png" });

    std::vector<std::vector<uint8_t>> buffers;
    for (const auto &filePath : paths)
    {
        std::string fileName = file::getFileName(filePath);

        const Optional<glm::vec2> coord = getCoordinates(fileName);
        if (coord.second)
        {
            coordinates.push_back(coord.first);
            buffers.push_back(ActivityManager::read(filePath));
        }
        else
        {
            LOGE("[%s] coordinates not found", fileName.c_str());
        }
    }

    empty = buffers.empty();
    if (empty)
    {
        coordinates.emplace_back(0.0f);
        texture = new TextureImage(
            device,
            0,
            VK_FORMAT_R8G8B8A8_UNORM,
            { 1, 1, 1 },
            1,
            2,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_SAMPLED_BIT,
            false);
    }
    else
    {
        texture = new TextureImage(device, buffers, false, false);
    }

    texture->pushFullView(VK_IMAGE_ASPECT_COLOR_BIT);
    texture->pushSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
}

Optional<glm::vec2> Gallery::getCoordinates(const std::string &fileName)
{
    Optional<glm::vec2> result(glm::vec2(), true);

    std::cmatch matches;
    std::regex regular(R"(\{([-+]?[0-9]*[,\.]?[0-9]+) +([-+]?[0-9]*[,\.]?[0-9]+)\})");

    if (std::regex_search(fileName.c_str(), matches, regular))
    {
        std::string latitude = matches[1].str();
        std::string longitude = matches[2].str();

        std::replace(latitude.begin(), latitude.end(), ',', '.');
        std::replace(longitude.begin(), longitude.end(), ',', '.');

        result.first.x = std::strtod(longitude.c_str(), nullptr);
        result.first.y = std::strtod(latitude.c_str(), nullptr);
    }
    else
    {
        regular = std::regex(R"(\w[\w\s-]+\w)");
        if (std::regex_search(fileName.c_str(), matches, regular))
        {
            std::string cityName = matches[0];
            std::transform(cityName.begin(), cityName.end(), cityName.begin(), ::tolower);
            const auto it = cities::COORDINATES.find(cityName);
            if (it != cities::COORDINATES.end())
            {
                result.first = it->second;
            }
            else
            {
                result.second = false;
            }
        }
    }

    return result;
}

float Gallery::loopDistance(glm::vec2 a, glm::vec2 b)
{
    float result;

    if (glm::abs(a.x - b.x) < 180.0f)
    {
        result = glm::distance(a, b);
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
        result = glm::distance(a, b);
    }

    return result;
}

float Gallery::calculateNearestDistance(glm::vec2 cameraCoordinates, uint32_t *outIndex)
{
    float nearestDistance = 360.0f;

    for (uint32_t i = 0; i < coordinates.size(); i++)
    {
        const float distance = loopDistance(cameraCoordinates, coordinates[i]);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            *outIndex = i;
        }
    }

    return nearestDistance;
}

float Gallery::calculateOpacity(float nearestDistance, float distanceLimit)
{
    const float transparencyDistance = distanceLimit / 2.0f;

    float opacity = 1.0f - (nearestDistance - transparencyDistance) / (distanceLimit - transparencyDistance);

    opacity = opacity > 1.0f ? 1.0f : opacity;
    opacity = std::pow(opacity, 0.5f);

    return opacity;
}

glm::vec3 Gallery::calculatePosition(glm::vec2 photoCoordinates)
{
    return earth->getRadius() * axis::rotate(
        -axis::X,
        glm::vec2(photoCoordinates.x + earth->getAngle(), photoCoordinates.y),
        nullptr);
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

glm::mat4 Gallery::calculateTransformation(glm::vec2 photoCoordinates, glm::vec2 cameraCoordinates)
{
    const glm::vec3 position = calculatePosition(photoCoordinates);
    const glm::vec3 direction = glm::normalize(camera->getPosition() - position);
    const glm::vec2 angleRadians(
        glm::radians(90.0f + cameraCoordinates.x + earth->getAngle()),
        glm::asin(direction.y));

    glm::mat4 transformation = glm::translate(glm::mat4(1.0f), position);
    transformation = glm::rotate(transformation, angleRadians.y, camera->getRight());
    transformation = glm::rotate(transformation, angleRadians.x, -axis::Y);
    transformation = glm::scale(transformation, calculateScale());

    return transformation;
}
