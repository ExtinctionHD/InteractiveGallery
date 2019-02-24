#include "Scene.h"
#include "sphere.h"

Scene::Scene(Device *device, VkExtent2D extent) : indexCount(sphere::INDICES.size())
{
    const Camera::Parameters cameraParameters{
        extent,
        90.0f,
        1.0f,
        50.0f
    };
    const Camera::Location cameraLocation{
        glm::vec3(0.0f, 0.0f, -20.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
    };
    camera = new Camera(device, cameraParameters, cameraLocation);

    controller = new Controller(cameraLocation.target, cameraLocation.position);

    const Lighting::Attributes lightingAttributes{
        glm::vec3(1.0f, 0.0f, 0.0f),
        8.0f,
        cameraLocation.position
    };
    lighting = new Lighting(device, lightingAttributes);

    sphereVertexBuffer = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sphere::VERTICES.size() * sizeof(Vertex));
    sphereIndexBuffer = new Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sphere::INDICES.size() * sizeof(uint32_t));
    sphereVertexBuffer->updateData(sphere::VERTICES.data());
    sphereIndexBuffer->updateData(sphere::INDICES.data());

    earth = new Earth(device, "textures/earth/2K/");

    LOGI("Scene created.");
}

Scene::~Scene()
{
    delete earth;
    delete sphereIndexBuffer;
    delete sphereVertexBuffer;
    delete lighting;
    delete controller;
    delete camera;
}

Buffer* Scene::getCameraBuffer() const
{
    return camera->getBuffer();
}

Buffer* Scene::getEarthTransformationBuffer() const
{
    return earth->getTransformationBuffer();
}

Buffer* Scene::getLightingBuffer() const
{
    return lighting->getBuffer();
}

std::vector<TextureImage *> Scene::getEarthTextures() const
{
    return earth->getTextures();
}

void Scene::handleMotion(glm::vec2 delta)
{
    controller->setDelta(delta);
}

void Scene::update()
{
    const float deltaSec = timer.getDeltaSec();

    controller->update(deltaSec);
    camera->update(controller->getLocation());

    earth->rotate(5.0f * deltaSec, glm::vec3(0.0f, -1.0f, 0.0f));

    logFps(deltaSec);
}

void Scene::resize(VkExtent2D newExtent)
{
    camera->resize(newExtent);
}

void Scene::drawSphere(VkCommandBuffer commandBuffer) const
{
    VkBuffer buffer = sphereVertexBuffer->get();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
    
    buffer = sphereIndexBuffer->get();
    vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Scene::logFps(float deltaSec)
{
    const float step = 1.0f;
    static int frameCount = 0;
    static float deltaSum = 0.0f;

    deltaSum += deltaSec;

    if (deltaSum < step)
    {
        frameCount++;
    }
    else
    {
        LOGD("FPS: %f.", frameCount / step);
        frameCount = 0;
        deltaSum = 0.0f;
    }
}
