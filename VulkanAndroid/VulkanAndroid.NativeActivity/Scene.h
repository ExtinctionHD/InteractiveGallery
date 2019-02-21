#pragma once
#include "Camera.h"
#include "Earth.h"
#include "Timer.h"
#include "Lighting.h"

class Scene
{
public:
    static const uint32_t BUFFER_COUNT = 3;
    static const uint32_t TEXTURE_COUNT = Earth::TextureType::COUNT;

    Scene(Device *device, VkExtent2D extent);

    ~Scene();

    Buffer* getCameraBuffer() const;

    Buffer* getEarthTransformationBuffer() const;

    Buffer* getLightingBuffer() const;

    std::vector<TextureImage*> getEarthTextures() const;

    void update();

    void resize(VkExtent2D newExtent);

    void drawSphere(VkCommandBuffer commandBuffer) const;

private:
    Camera *camera;

    Lighting *lighting;

    Buffer *sphereVertexBuffer;

    Buffer *sphereIndexBuffer;

    uint32_t indexCount;

    Timer timer;

    Earth *earth;

    static void logFps(float deltaSec);
};

