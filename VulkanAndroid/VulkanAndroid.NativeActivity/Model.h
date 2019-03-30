#pragma once
#include "Buffer.h"
#include "TextureImage.h"

class Model
{
public:
    virtual ~Model();

    virtual std::vector<DescriptorInfo> getTextureInfos() const = 0;

    virtual std::vector<DescriptorInfo> getUniformBufferInfos() const = 0;

    DescriptorInfo getTransformationBufferInfo() const;

    glm::mat4 getTransformation() const;

    void setTransformation(glm::mat4 transformation);

protected:
    Model(Device *device);

private:
    glm::mat4 transformation;

    Buffer *transformationBuffer;
};

