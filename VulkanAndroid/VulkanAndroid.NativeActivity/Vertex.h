#pragma once

struct Vertex
{
	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec2 uv = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);	
	glm::vec3 tangent = glm::vec3(0.0f);

	static VkVertexInputBindingDescription getBindingDescription(uint32_t binding);

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, uint32_t locationOffset);
};

