#pragma once
#include "Camera.h"
#include "Earth.h"
#include "Timer.h"
#include "Lighting.h"
#include "Controller.h"
#include "Skybox.h"
#include "Clouds.h"
#include "Gallery.h"

class Scene
{
public:
    enum class ModelId
    {
        EARTH,
        CLOUDS,
        SKYBOX,
        GALLERY,
        COUNT,
    };

    static const uint32_t BUFFER_COUNT = 7;
    static const uint32_t TEXTURE_COUNT = EARTH_TEXTURE_TYPE_COUNT + 4;

    Scene(Device *device, VkExtent2D extent);

    ~Scene();

    Buffer* getCameraBuffer() const;

    DescriptorInfo getLightingBufferInfo() const;

    std::vector<DescriptorInfo> getModelTextureInfos(ModelId id);

    std::vector<DescriptorInfo> getModelUniformBufferInfo(ModelId id);

    DescriptorInfo getModelTransformationBufferInfo(ModelId id);

    void handleMotion(glm::vec2 delta);

    void handleZoom(float delta);

    void skipTime();

    void update();

    void resize(VkExtent2D newExtent);

    void drawSphere(VkCommandBuffer commandBuffer) const;

    void drawCube(VkCommandBuffer commandBuffer) const;

    void drawCard(VkCommandBuffer commandBuffer) const;

private:
    enum MeshBufferType
    {
        SPHERE_VERTEX_BUFFER,
        SPHERE_INDEX_BUFFER,
        CUBE_VERTEX_BUFFER,
        CUBE_INDEX_BUFFER,
        CARD_VERTEX_BUFFER,
        CARD_INDEX_BUFFER,
        MESH_BUFFER_COUNT
    };

    Camera *camera;

    Controller *controller;

    Lighting *lighting;

    std::vector<Buffer*> meshBuffers;

    Timer timer;

    std::vector<Model*> models;

    Earth *earth;

    Clouds *clouds;

    Skybox *skybox;

    Gallery *gallery;

    void initMeshes(Device *device);

    static void logFps(float deltaSec);
};

