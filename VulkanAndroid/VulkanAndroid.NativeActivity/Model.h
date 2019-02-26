#pragma once
#include "Buffer.h"

class Model
{
public:
    virtual ~Model();

    glm::mat4 getTransformation() const;

    Buffer* getTransformationBuffer() const;

    void rotate(float angle, glm::vec3 axis);

    void setPosition(glm::vec3 position);

protected:
    Model(Device *device);

    void setTransformation(glm::mat4 transformation);

private:
    glm::mat4 transformation;

    Buffer *transformationBuffer;
};

