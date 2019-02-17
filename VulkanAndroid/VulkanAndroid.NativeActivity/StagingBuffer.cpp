#include "StagingBuffer.h"

StagingBuffer::StagingBuffer(Device *device, VkDeviceSize size) : device(device), size(size)
{
	createBuffer(
		device,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		&stagingMemory);
}

StagingBuffer::~StagingBuffer()
{
	vkFreeMemory(device->get(), stagingMemory, nullptr);
	vkDestroyBuffer(device->get(), stagingBuffer, nullptr);
}

VkBuffer StagingBuffer::get() const
{
    return stagingBuffer;
}

VkDeviceSize StagingBuffer::getSize() const
{
    return size;
}

void StagingBuffer::updateData(const void *data, VkDeviceSize offset, VkDeviceSize dataSize)
{
    if (dataSize == VkDeviceSize(-1))
    {
        dataSize = size - offset;
    }

	LOGA(offset + dataSize <= size, "Size of updated data can't be bigger than buffer size");

	void *bufferData;
	vkMapMemory(device->get(), stagingMemory, offset, dataSize, 0, &bufferData);
	memcpy(bufferData, data, dataSize);
	vkUnmapMemory(device->get(), stagingMemory);
}

void StagingBuffer::copyToImage(VkImage image, std::vector<VkBufferImageCopy> regions) const
{
    VkCommandBuffer commandBuffer = device->beginOneTimeCommands();

	vkCmdCopyBufferToImage(
        commandBuffer,
        stagingBuffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		uint32_t(regions.size()),
        regions.data());

	device->endOneTimeCommands(commandBuffer);
}

void StagingBuffer::createBuffer(
    Device *device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *buffer,
    VkDeviceMemory *memory)
{
	VkBufferCreateInfo createInfo{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,							
		0,									
		size,								
		usage,								
		VK_SHARING_MODE_EXCLUSIVE,			
		0,									
		nullptr,							
	};

    CALL_VK(vkCreateBuffer(device->get(), &createInfo, nullptr, buffer));

	allocateMemory(device, *buffer, memory, properties);

	vkBindBufferMemory(device->get(), *buffer, *memory, 0);
}

void StagingBuffer::allocateMemory(
    Device *device,
    VkBuffer buffer,
    VkDeviceMemory *memory,
    VkMemoryPropertyFlags properties)
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device->get(), buffer, &memRequirements);

    const uint32_t memoryTypeIndex = device->findMemoryTypeIndex(
		memRequirements.memoryTypeBits,
		properties);

	VkMemoryAllocateInfo allocInfo{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		nullptr,				
		memRequirements.size,	
		memoryTypeIndex,		
	};

    CALL_VK(vkAllocateMemory(device->get(), &allocInfo, nullptr, memory));
}
