#pragma once

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>

#include "Vertex.h"

class Mesh
{
public:
	Mesh() = default;

	void lazyInit(std::string, VkPhysicalDevice, VkDevice);

	std::vector<Vertex> getVertices() const { return mVertices; }
	std::vector<uint32_t> getIndices() const { return mIndices; }

private:
	void loadModel();
	void createVertexBuffer();
	void createIndexBuffer();

	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;

	std::string mModelDir;
};

#endif // MESH_H