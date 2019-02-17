#pragma once
#include "Device.h"

// buffer that can be mapped into host memory
class StagingBuffer
{
public:
	StagingBuffer(Device *device, VkDeviceSize size);

	virtual ~StagingBuffer();

    VkBuffer get() const;

    VkDeviceSize getSize() const;

	virtual void updateData(const void *data, VkDeviceSize offset = 0, VkDeviceSize dataSize = VkDeviceSize(-1));

	void copyToImage(VkImage image, std::vector<VkBufferImageCopy> regions) const;

protected:
	Device *device;

	VkBuffer stagingBuffer;

	VkDeviceSize size;

	VkDeviceMemory stagingMemory;

	static void createBuffer(
		Device *device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer *buffer,
		VkDeviceMemory *memory);

	static void allocateMemory(
        Device *device,
        VkBuffer buffer,
        VkDeviceMemory *memory,
        VkMemoryPropertyFlags properties);
};

