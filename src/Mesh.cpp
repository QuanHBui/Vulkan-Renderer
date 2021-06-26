#include "Mesh.h"

#include <unordered_map>
#include <stdexcept>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void Mesh::lazyInit(std::string modelDir, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
	mModelDir = modelDir;

	loadModel();
	//createVertexBuffer();
	//createIndexBuffer();
}

void Mesh::loadModel()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	// The whole thing fails if the mtl file is not found. Kinda weird!
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, mModelDir.c_str()));
	{
		//throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto &shape : shapes)
	{
		for (const auto &index : shape.mesh.indices)
		{
			Vertex vertex{};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				// The currente size of the vertex buffer is the current index of an unique vertex
				uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
				mVertices.push_back(vertex);
			}

			mIndices.push_back(uniqueVertices[vertex]);
		}
	}
}

void Mesh::createVertexBuffer()
{
	//VkDeviceSize bufferSize = sizeof(mVertices[0]) * mVertices.size();

	//VulkanBuffer stagingBuffer{
	//	mLogicalDevice,
	//	mPhysicalDevice,
	//	bufferSize,
	//	VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	//};

	//mVertexBuffer.lazyInit(
	//	mLogicalDevice,
	//	mPhysicalDevice,
	//	bufferSize,
	//	VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	//	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	//);
}

void Mesh::createIndexBuffer()
{

}