#pragma once
#include "StagingBuffer.h"

// buffer with device local memory 
class Buffer : public StagingBuffer
{
public:
	Buffer(Device *device, VkBufferUsageFlags usage, VkDeviceSize size);
	~Buffer();

	VkBuffer get() const;

    void updateData(const void *data);

	void updateData(const void *data, VkDeviceSize offset, VkDeviceSize dataSize) override;

private:
	VkBuffer buffer;

	VkDeviceMemory memory;
};

