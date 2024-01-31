#pragma once
#include <string>
#include <vector>
#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include "Graphics/Lights.h"

class DXRootSignature;
class DXPipeline;

class Camera;
class Model;

// TODO: For now window size stuff is handled in Renderer
// in the future move it with serialization to Engine.
class Renderer
{
public:
	Renderer(const std::wstring& applicationName);

	void Update(float deltaTime);
	void Render();

	void Resize();
	void AddModel(const std::string& filePath);

private:
	void InitializeImGui();

	void UpdateLightBuffer();

private:
	DXRootSignature* rootSignature;
	DXPipeline* pipeline;
	Camera* camera; 

	std::vector<Model*> models;

	LightData lights;
	ComPtr<ID3D12Resource> lightBuffer;
	int lightCBVIndex = -1;

	float elaspedTime = 0.0f;
	unsigned int width = 1080;
	unsigned int height = 720;

	friend class Editor;
};