#include "Framework/Editor.h"
#include "Framework/Scene.h"

#include "Utilities/Logger.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Graphics/Texture.h"

#include <d3d12.h>
#include <imgui.h>
#include <filesystem>

Editor::Editor(Scene* scene) : scene(scene)
{
	ImGuiStyleSettings();
	LoadModelFilePaths("Assets/Models/", "Assets/Models/");

	std::string modelCount = std::to_string(modelFilePaths.size());
	LOG("Found " + modelCount + " usable glTF(s) inside of 'Assets/Models'");
}

void Editor::Update(float deltaTime)
{
	this->deltaTime = deltaTime;

	ModelSelectionWindow();
	StatisticsWindow();
	LightsWindow();

	HierachyWindow();
	DetailsWindow();
}

void Editor::SetScene(Scene* newScene)
{
	scene = newScene;
}

void Editor::ModelSelectionWindow()
{
	ImGui::Begin("Model Selection");

	std::string& selectedPath = displayNames[currentModelID];
	if(ImGui::BeginCombo("Model File", selectedPath.c_str()))
	{
		for(int i = 0; i < displayNames.size(); i++)
		{
			bool isSelected = currentModelID == i;

			if(ImGui::Selectable(displayNames[i].c_str(), isSelected))
			{
				currentModelID = i;
			}

			if(isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if(ImGui::Button("Load Model"))
	{
		scene->AddModel(modelFilePaths[currentModelID]);
	}

	ImGui::End();
}

void Editor::StatisticsWindow()
{
	ImGui::Begin("Statistics");
	ImGui::SeparatorText("Stats");

	ImGui::Text("FPS: %i", int(1.0f / deltaTime));
	ImGui::End();
}

void Editor::LightsWindow()
{
	ImGui::Begin("Lights");

	if(ImGui::Button("Add Light"))
	{
		if(scene->lights.activePointLights < MAX_AMOUNT_OF_LIGHTS)
		{
			scene->lights.activePointLights++;
			scene->lightsEdited = true;
		}
		else
		{
			LOG(Log::MessageType::Debug, "You are already at the limit of lights")
		}
	}

	for(int i = 0; i < scene->lights.activePointLights; i++)
	{
		ImGui::PushID(i);

		PointLight& pointLight = scene->lights.pointLights[i];

		std::string name = "Light - " + std::to_string(i);
		ImGui::SeparatorText(name.c_str());

		if(ImGui::DragFloat3("Position", &pointLight.Position[0], 0.01f)) { scene->lightsEdited = true; }
		if(ImGui::ColorEdit3("Color", &pointLight.Color[0])) { scene->lightsEdited = true; }
		if(ImGui::DragFloat("Intensity", &pointLight.Intensity, 0.05f, 0.0f, 1000.0f)) { scene->lightsEdited = true; }

		ImGui::PopID();
	}

	ImGui::End();
}

void Editor::HierachyWindow()
{
	ImGui::Begin("Scene Hierachy");
	ImGui::Indent(8.0f);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));

	for(int i = 0; i < scene->models.size(); i++)
	{
		ImGui::PushID(i);
		ImGui::Bullet();

		Model* model = scene->models[i];
		bool isSelected = model == hierachySelectedModel;
		if(ImGui::Selectable(model->Name.c_str(), isSelected))
		{
			hierachySelectedModel = model;
		}

		if(isSelected)
		{
			ImGui::SetItemDefaultFocus();
		}

		ImGui::PopID();
	}

	ImGui::PopStyleColor();

	ImGui::Unindent(8.0f);
	ImGui::End();
}

void Editor::DetailsWindow()
{
	if(!hierachySelectedModel)
	{
		return;
	}

	Model* model = hierachySelectedModel;
	Mesh* mesh = model->GetMesh(0);
	Material& material = model->GetMesh(0)->Material;

	ImGui::Begin("Details");
	ImGui::SeparatorText(model->Name.c_str());

	ImGui::SeparatorText("Transform");
	ImGui::DragFloat3("Position:", &model->Transform.Position[0], 0.05f);
	ImGui::DragFloat3("Rotation:", &model->Transform.Rotation[0], 0.05f);
	ImGui::DragFloat3("Scale:", &model->Transform.Scale[0], 0.01f, 0.0f, 10000.0f);
	ImGui::Separator();

	ImGui::SeparatorText("Material Settings");
	bool materialUpdated = false;

	bool useTextures = material.useTextures;
	if(ImGui::Checkbox("Use Textures", &useTextures))
	{
		material.useTextures = useTextures;
		materialUpdated = true;
	}

	if(!useTextures)
	{
		if(ImGui::ColorEdit3("Color", &material.Color[0])) { materialUpdated = true; }
		if(ImGui::SliderFloat("Metallic", &material.Metallic, 0.0f, 1.0f)) { materialUpdated = true; }
		if(ImGui::SliderFloat("Roughness", &material.Roughness, 0.0f, 1.0f)) { materialUpdated = true; }
	}

	if(ImGui::SliderInt("Occlusion Channel", &material.oChannel, 0, 2)) { materialUpdated = true; }
	if(ImGui::SliderInt("Roughness Channel", &material.rChannel, 0, 2)) { materialUpdated = true; }
	if(ImGui::SliderInt("Metallic Channel", &material.mChannel, 0, 2)) { materialUpdated = true; }

	ImGui::Separator();

	if(materialUpdated)
	{
		std::vector<Mesh*> meshes = model->GetMeshes();

		for(Mesh* mesh : meshes)
		{
			mesh->Material.oChannel = material.oChannel;
			mesh->Material.rChannel = material.rChannel;
			mesh->Material.mChannel = material.mChannel;

			mesh->UpdateMaterialData();
		}
	}

	if(mesh->HasTextures())
	{
		ImGui::SeparatorText("Textures");
		TextureColumnHighlight(mesh->albedoTexture, "Albedo");
		TextureColumnHighlight(mesh->normalTexture, "Normal");
		TextureColumnHighlight(mesh->metallicRoughnessTexture, "Metallic Roughness");
		TextureColumnHighlight(mesh->occlusionTexture, "Oclussion");
		TextureColumnHighlight(mesh->emissiveTexture, "Emissive");
		ImGui::Separator();
	}

	ImGui::End();
}

void Editor::TextureColumnHighlight(Texture* texture, std::string name)
{
	ImGui::Separator();
	ImGui::Columns(2);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = texture->GetSRV();
	ImGui::Image((ImTextureID)gpuHandle.ptr, ImVec2(32, 32));
	ImGui::NextColumn();
	ImGui::Text(name.c_str());

	ImGui::Columns(1);
	ImGui::Separator();
}

void Editor::LoadModelFilePaths(std::string path, std::string originalPath)
{
	for(const auto& file : std::filesystem::directory_iterator(path))
	{
		if(file.is_directory())
		{
			LoadModelFilePaths(file.path().string(), originalPath);
		}

		std::string filePath = file.path().string();
		std::string fileType = filePath.substr(filePath.find_last_of('.') + 1, filePath.size());

		if(fileType == "gltf")
		{
			displayNames.push_back(filePath.substr(filePath.find_last_of('\\') + 1));
			modelFilePaths.push_back(filePath.c_str());
		}
	}
}

void Editor::ImGuiStyleSettings()
{
	ImGuiIO& io = ImGui::GetIO();

	// Fonts //	
	io.FontDefault = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Roboto-Regular.ttf", 13.f);
	io.Fonts->AddFontFromFileTTF("Assets/Fonts/Roboto-Bold.ttf", 13.f);

	baseFont = ImGui::GetFont();
	boldFont = io.Fonts->Fonts[1];

	// Style //
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScrollbarRounding = 2;
	style.ScrollbarSize = 12;
	style.WindowRounding = 3;
	style.WindowBorderSize = 0.0f;
	style.WindowTitleAlign = ImVec2(0.0, 0.5f);
	style.WindowPadding = ImVec2(5, 1);
	style.ItemSpacing = ImVec2(12, 5);
	style.FrameBorderSize = 0.5f;
	style.FrameRounding = 3;
	style.GrabMinSize = 5;

	// Color Wheel //
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR |
		ImGuiColorEditFlags_PickerHueBar);

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.761, 0.761, 0.761, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.118f, 0.118f, 0.118f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.21f, 0.21f, 0.21f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.85f, 0.48f, 0.21f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.85f, 0.48f, 0.21f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.96f, 0.72f, 0.55f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}