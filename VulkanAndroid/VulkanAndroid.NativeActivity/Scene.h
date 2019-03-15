#pragma once
#include "Camera.h"
#include "Earth.h"
#include "Timer.h"
#include "Lighting.h"
#include "Controller.h"
#include "Skybox.h"
#include "Clouds.h"
#include "PhotoCard.h"

class Scene
{
public:
    static const uint32_t BUFFER_COUNT = 5;
    static const uint32_t TEXTURE_COUNT = EARTH_TEXTURE_TYPE_COUNT + 3;

    Scene(Device *device, VkExtent2D extent);

    ~Scene();

    Buffer* getCameraBuffer() const;

    Buffer* getEarthTransformationBuffer() const;

    Buffer* getCloudsTransformationBuffer() const;

    Buffer* getSkyboxTransformationBuffer() const;

    Buffer* getLightingBuffer() const;

    std::vector<TextureImage*> getEarthTextures() const;

    TextureImage* getCloudsTexture() const;

    TextureImage* getSkyboxTexture() const;

    void handleMotion(glm::vec2 delta);

    void skipTime();

    void update();

    void resize(VkExtent2D newExtent);

    void drawSphere(VkCommandBuffer commandBuffer) const;

    void drawCube(VkCommandBuffer commandBuffer) const;

private:
    enum MeshBufferType
    {
        SPHERE_VERTEX_BUFFER,
        SPHERE_INDEX_BUFFER,
        CUBE_VERTEX_BUFFER,
        CUBE_INDEX_BUFFER,
        MESH_BUFFER_COUNT
    };

    Camera *camera;

    Controller *controller;

    Lighting *lighting;

    std::vector<Buffer*> meshBuffers;

    Timer timer;

    Earth *earth;

    Clouds *clouds;

    Skybox *skybox;

    PhotoCard *photoCard;

    void initMeshes(Device *device);

    static void logFps(float deltaSec);
};

