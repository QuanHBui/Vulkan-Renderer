#pragma once

#ifndef VERTEX_H
#define VERTEX_H

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex &other) const
	{
		return position == other.position && color == other.color && texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const &vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position)
				^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1)
				^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

#endif // VERTEX_H