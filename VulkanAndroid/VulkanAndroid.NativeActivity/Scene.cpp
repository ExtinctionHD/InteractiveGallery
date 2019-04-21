#include "Scene.h"
#include "sphere.h"
#include "cube.h"
#include "card.h"

Scene::Scene(Device *device, VkExtent2D extent)
{
    const Camera::Parameters cameraParameters{
        extent,
        90.0f,
        1.0f,
        60.0f
    };
    const Camera::Location cameraLocation{
        glm::vec3(0.0f, 0.0f, -25.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
    };
    camera = new Camera(device, cameraParameters, cameraLocation);

    controller = new Controller(cameraLocation.target, cameraLocation.position);

    const Lighting::Attributes lightingAttributes{
        glm::vec3(1.0f, 0.0f, 0.0f),
        20.0f,
        cameraLocation.position,
        8.0f,
        0.02f,
    };
    lighting = new Lighting(device, lightingAttributes);


    earth = new Earth(device, "textures/earth/2K/");
    clouds = new Clouds(device, "textures/earth/2K/");
    skybox = new Skybox(device, "textures/Stars/");
    gallery = new Gallery(device, "Gallery/", earth, camera, controller);

    models.resize(uint32_t(ModelId::COUNT));
    models[uint32_t(ModelId::EARTH)] = earth;
    models[uint32_t(ModelId::CLOUDS)] = clouds;
    models[uint32_t(ModelId::SKYBOX)] = skybox;
    models[uint32_t(ModelId::GALLERY)] = gallery;

    initMeshes(device);

    LOGI("Scene created.");
}

Scene::~Scene()
{
    for (auto &buffer : meshBuffers)
    {
        delete buffer;
    }

    for (auto &model : models)
    {
        delete model;
    }

    delete lighting;
    delete controller;
    delete camera;
}

Buffer* Scene::getCameraBuffer() const
{
    return camera->getBuffer();
}

DescriptorInfo Scene::getLightingBufferInfo() const
{
    return lighting->getBuffer()->getUniformBufferInfo();
}

std::vector<DescriptorInfo> Scene::getModelTextureInfos(ModelId id)
{
    return models[uint32_t(id)]->getTextureInfos();
}

std::vector<DescriptorInfo> Scene::getModelUniformBufferInfo(ModelId id)
{
    return models[uint32_t(id)]->getUniformBufferInfos();
}

DescriptorInfo Scene::getModelTransformationBufferInfo(ModelId id)
{
    return models[uint32_t(id)]->getTransformationBufferInfo();
}

void Scene::handleMotion(glm::vec2 delta)
{
    controller->setMotionDelta(delta);
}

void Scene::handleZoom(float delta)
{
    controller->setZoomDelta(delta);
}

void Scene::skipTime()
{
    timer.getDeltaSec();
}

void Scene::update()
{
    const float deltaSec = timer.getDeltaSec();

    controller->update(deltaSec);
    camera->update(controller->getLocation());
    lighting->update(camera->getPosition());

    earth->rotate(2.0f * deltaSec);
    clouds->setEarthTransformation(earth->getTransformation());
    skybox->setTransformation(translate(glm::mat4(1.0f), camera->getPosition()));
    gallery->update();

#ifndef NDEBUG
    logFps(deltaSec);
#endif
}

void Scene::activateGallery()
{
    gallery->activate();
}

void Scene::resize(VkExtent2D newExtent)
{
    camera->resize(newExtent);
}

void Scene::drawSphere(VkCommandBuffer commandBuffer) const
{
    VkBuffer buffer = meshBuffers[SPHERE_VERTEX_BUFFER]->get();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
    
    buffer = meshBuffers[SPHERE_INDEX_BUFFER]->get();
    vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdDrawIndexed(commandBuffer, sphere::INDICES.size(), 1, 0, 0, 0);
}

void Scene::drawCube(VkCommandBuffer commandBuffer) const
{
    VkBuffer buffer = meshBuffers[CUBE_VERTEX_BUFFER]->get();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);

    buffer = meshBuffers[CUBE_INDEX_BUFFER]->get();
    vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, cube::INDICES.size(), 1, 0, 0, 0);
}

void Scene::drawCard(VkCommandBuffer commandBuffer) const
{
    VkBuffer buffer = meshBuffers[CARD_VERTEX_BUFFER]->get();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);

    buffer = meshBuffers[CARD_INDEX_BUFFER]->get();
    vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, card::INDICES.size(), 1, 0, 0, 0);
}

void Scene::initMeshes(Device *device)
{
    meshBuffers.resize(MESH_BUFFER_COUNT);

    meshBuffers[SPHERE_VERTEX_BUFFER] = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sphere::VERTICES.size() * sizeof(Vertex));
    meshBuffers[SPHERE_INDEX_BUFFER] = new Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sphere::INDICES.size() * sizeof(uint32_t));
    meshBuffers[SPHERE_VERTEX_BUFFER]->updateData(sphere::VERTICES.data());
    meshBuffers[SPHERE_INDEX_BUFFER]->updateData(sphere::INDICES.data());

    meshBuffers[CUBE_VERTEX_BUFFER] = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, cube::VERTICES.size() * sizeof(Position));
    meshBuffers[CUBE_INDEX_BUFFER] = new Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, cube::INDICES.size() * sizeof(uint32_t));
    meshBuffers[CUBE_VERTEX_BUFFER]->updateData(cube::VERTICES.data());
    meshBuffers[CUBE_INDEX_BUFFER]->updateData(cube::INDICES.data());

    meshBuffers[CARD_VERTEX_BUFFER] = new Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, card::VERTICES.size() * sizeof(PositionUv));
    meshBuffers[CARD_INDEX_BUFFER] = new Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, card::INDICES.size() * sizeof(uint32_t));
    meshBuffers[CARD_VERTEX_BUFFER]->updateData(card::VERTICES.data());
    meshBuffers[CARD_INDEX_BUFFER]->updateData(card::INDICES.data());
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
