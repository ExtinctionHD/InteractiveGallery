#include "Scene.h"
#include "sphere.h"

Scene::Scene(Device *device, VkExtent2D extent) : indexCount(sphere::INDICES.size())
{
    const Camera::Attributes cameraAttributes{
        extent,
        glm::vec3(0.0f, 0.0f, -20.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        90.0f,
        1.0f,
        50.0f
    };
    camera = new Camera(device, cameraAttributes);

    const Lighting::Attributes lightingAttributes{
        glm::vec3(1.0f, 0.0f, 0.0f),
        4.0f,
        cameraAttributes.position
    };
    lighting = new Lighting(device, lightingAttributes);

    sphereVertexBuffer = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sphere::VERTICES.size() * sizeof(Vertex));
    sphereIndexBuffer = new Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sphere::INDICES.size() * sizeof(uint32_t));
    sphereVertexBuffer->updateData(sphere::VERTICES.data());
    sphereIndexBuffer->updateData(sphere::INDICES.data());

    earth = new Earth(device, "textures/earth/2K/");
    //earth->rotate(90.0f, glm::vec3(0.0f, -1.0f, 0.0f));

    LOGI("Scene created.");
}

Scene::~Scene()
{
    delete earth;
    delete sphereIndexBuffer;
    delete sphereVertexBuffer;
    delete lighting;
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

void Scene::update()
{
    const float deltaSec = timer.getDeltaSec();

    earth->rotate(20.0f * deltaSec, glm::vec3(0.0f, -1.0f, 0.0f));

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
    static int frameCount = 0;
    static float deltaSum = 0.0f;

    deltaSum += deltaSec;

    if (deltaSum < 1.0f)
    {
        frameCount++;
    }
    else
    {
        LOGD("FPS: %d.", frameCount);
        frameCount = 0;
        deltaSum = 0.0f;
    }
}
