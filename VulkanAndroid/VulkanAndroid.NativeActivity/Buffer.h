#pragma once
#include "StagingBuffer.h"
#include "DescriptorInfo.h"

// buffer with device local memory 
class Buffer : public StagingBuffer
{
public:
	Buffer(Device *device, VkBufferUsageFlags usage, VkDeviceSize size);
	~Buffer();

	VkBuffer get() const;

    DescriptorInfo getUniformBufferInfo() const;

	void updateData(const void *data, VkDeviceSize offset = 0, VkDeviceSize dataSize = VkDeviceSize(-1)) override;

private:
	VkBuffer buffer;

	VkDeviceMemory memory;
};

