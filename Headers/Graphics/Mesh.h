#pragma once

#include <vector>
#include <string>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include "Framework/Mathematics.h"
#include "tiny_gltf.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec3 Color;
	glm::vec2 TexCoord;
};

struct Material
{
	int hasAlbedo;
	int hasNormal;
	int hasMetallicRoughness;
	int hasOcclusion;
	int hasEmissive;

	int oChannel = 0;
	int rChannel = 1;
	int mChannel = 2;

	// Customize //
	int useTextures = 1;
	glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
	float Metallic = 0.0f;
	float Roughness = 0.0f;

	// GPU Memory alignment //
	double stub[25];
};

class Texture;

class Mesh
{
public:
	Mesh(tinygltf::Model& model, tinygltf::Primitive& primitive, glm::mat4& transform);
	Mesh(Vertex* vertices, unsigned int vertexCount, unsigned int* indices, unsigned int indexCount);

	void UpdateMaterialData();

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView();
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView();
	const CD3DX12_GPU_DESCRIPTOR_HANDLE GetMaterialView();
	const unsigned int GetIndicesCount();

	bool HasTextures();
	unsigned int GetTextureID();

private:
	void LoadAttribute(tinygltf::Model& model, tinygltf::Primitive& primitive, const std::string& attributeType);
	void LoadIndices(tinygltf::Model& model, tinygltf::Primitive& primitive);
	void LoadMaterial(tinygltf::Model& model, tinygltf::Primitive& primitive);
	void LoadTexture(tinygltf::Model& model, Texture** texture, int textureID, int& materialCheck);

	void GenerateTangents();

	void ApplyNodeTransform(const glm::mat4& transform);
	void UploadBuffers();

public:
	std::string Name;
	Material Material;

	// Texture & Material Data //
	Texture* albedoTexture = nullptr;
	Texture* normalTexture = nullptr;
	Texture* metallicRoughnessTexture = nullptr;
	Texture* occlusionTexture = nullptr;
	Texture* emissiveTexture = nullptr;

private:
	// Vertex & Index Data //
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int indicesCount = 0;


	bool hasTextures = false;

	int materialCBVIndex = -1;
	ComPtr<ID3D12Resource> materialBuffer;
};