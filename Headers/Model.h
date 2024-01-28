#pragma once

#include <string>
#include <vector>
#include "tiny_gltf.h"
#include "DXUtilities.h"

class Mesh;

class Model
{
public:
	Model(const std::string& filePath);

	void Draw();

private:
	void TraverseRootNodes(tinygltf::Model& model);
	void TraverseChildNodes(tinygltf::Model& model, tinygltf::Node& node, matrix& parentMatrix);

	std::vector<Mesh*> meshes;
};