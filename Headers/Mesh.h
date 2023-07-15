#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <vector>
#include <glm.hpp>
#include "Transform.h"
#include <tiny_gltf.h>

using namespace Microsoft::WRL;

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Color;
};

class Mesh
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int> indices);
	Mesh(tinygltf::Model& model, tinygltf::Mesh& mesh);

	void SetAndDraw();
	void ClearIntermediateBuffers();

	Transform transform;

private:
	void LoadVertices(tinygltf::Model& model, tinygltf::Primitive& primitive);
	void LoadIndices(tinygltf::Model& model, tinygltf::Primitive& primitive);

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	ComPtr<ID3D12Resource> intermediateVertexBuffer;
	ComPtr<ID3D12Resource> intermediateIndexBuffer;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	unsigned int amountOfIndices;
};