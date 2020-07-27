#include "Vertex.h"

/**
 * A vertex binding specifies the number of bytes between data entries and whether to :
 *  (1) move to the next data entry after each vertex OR
 *  (2) after each instance
 * Since we are not doing instanced rendering, we are going to use (1)
 *
 * This piece of information is used to describe to the GPU how to read
 *  the data per vertex, as opposed to VkVertexInputAttributeDescription
 *  below, which is per attribute.
 */
VkVertexInputBindingDescription Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	// We are not using instanced rendering so this is
																//  going to per-vertex data

	return bindingDescription;
}

/**
 * We have 2 attribute description objects because we have 2 attributes, position and color.
 *
 * This struct, VkVertexInputAttributeDescription, shows the GPU how to read
 *  data per attribute.
 */
std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

	// Position attibute
	attributeDescriptions[0].binding = 0;		// Which binding per-vertex data from
	attributeDescriptions[0].location = 0;		// location directive specified in vertex shader
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;		// vec3 format. Vulkan uses the same enumeration as color format for some reason
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	// Color attribute
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;
}