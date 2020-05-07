#pragma once
#include "Drawable.h"
#include "BindableCommon.h"
#include "Vertex.h"
#include <optional>
#include "ConstantBuffers.h"
#include <type_traits>
#include <filesystem>
#include "imgui/imgui.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Bind;

class ModelException : public RedSkyException {
public:
	ModelException(int line, const char* file, std::string note) noexcept;
	const char* what() const noexcept override;
	const char* GetType() const noexcept override;
	const std::string& GetNote() const noexcept;
private:
	std::string note;
};


//Mesh is a drawable object
class Mesh : public Drawable {
public:
	//takes in a vector of bind ables //allows the user to pass in what bindables should comprise the mesh
	Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bindable>> bindPtrs);

	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
	DirectX::XMMATRIX GetTransformXM() const noexcept override { return DirectX::XMLoadFloat4x4(&transform); } //This is called in the Drawable::Draw method

private:
	mutable DirectX::XMFLOAT4X4 transform;
};

class Node {
	friend class Model;
public:
	struct PSMaterialConstant_DiffNormSpec {
		BOOL normalMapEnabled = TRUE;
		BOOL specularMapEnabled = TRUE;
		BOOL hasGlossMap = FALSE;
		float specularPower = 3.1f;
		DirectX::XMFLOAT3 specularColor = { 0.75f,0.75f,0.75f };
		float specularMapWeight = 0.671f;
	};
	struct PSMaterialConstant_DiffSpec {
		float specularPowerConst;
		BOOL hasGloss;
		float specularMapWeight;
		float padding;
	};
	struct PSMaterialConstant_DiffNorm {
		float specularIntensity = 1.0f;
		float specularPower = 1.0f;
		BOOL normalMapEnabled = TRUE;
		float padding[1];
	};
	struct PSMaterialConstant_Diff {
		float specularIntensity = 25.0f;
		float specularPower = 1.0f;
		float padding[2];
	};
	struct PSMaterialConstant_Notex {
		DirectX::XMFLOAT4 materialColor = { 0.447970f, 0.327254f, 0.176283f,1.0f };
		DirectX::XMFLOAT4 specularColor = { 0.65f,0.65f,0.65f,1.0f };
		float specularPower = 120.0f;
		float padding[3];
	};
public:
	Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in) noxnd;

	int GetID() const noexcept { return id; }
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransforms) const noxnd;
	void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
	const DirectX::XMFLOAT4X4& GetAppliedTransform() const noexcept;
	void ShowTree(Node*& pSelectedNode) const noexcept;
	std::string GetName() const noexcept { return name; }

	//TODO: Add DiffNorm and Diff menus to this system
	template<class T>
	bool SpawnMaterialControlPanel(Graphics& gfx, T& c) {
		if (meshPtrs.empty()) {
			return false;
		}

		if constexpr (std::is_same<T, PSMaterialConstant_DiffNormSpec>::value) {
			if (auto pcb = meshPtrs.front()->QueryBindable<Bind::PixelConstantBuffer<T>>()) {
				ImGui::Text("Material");

				bool normalMapEnabled = (bool)c.normalMapEnabled;
				bool specularMapEnabled = (bool)c.specularMapEnabled;
				bool hasGlossMap = (bool)c.hasGlossMap;

				ImGui::Checkbox("Normal Map", &normalMapEnabled);
				ImGui::Checkbox("Specular Map", &specularMapEnabled);
				ImGui::Checkbox("Gloss Alpha", &specularMapEnabled);

				ImGui::SliderFloat("Specular Weight", &c.specularMapWeight, 0.0f, 2.0f);
				ImGui::SliderFloat("Specular Power", &c.specularPower, 0.0f, 1000.0f, "%f", 5.0f);

				ImGui::ColorPicker3("Specular Color", reinterpret_cast<float*>(&c.specularColor));

				c.normalMapEnabled = normalMapEnabled ? TRUE : FALSE;
				c.specularMapEnabled = specularMapEnabled ? TRUE : FALSE;
				c.hasGlossMap = hasGlossMap ? TRUE : FALSE;

				pcb->Update(gfx, c);
				return true;
			}
		}
		else if constexpr (std::is_same<T, PSMaterialConstant_DiffNorm>::value) {
			if (auto pcb = meshPtrs.front()->QueryBindable<Bind::PixelConstantBuffer<T>>()) {
				ImGui::Text("Material");

				bool normalMapEnabled = (bool)c.normalMapEnabled;

				ImGui::Checkbox("Normal Map", &normalMapEnabled);

				ImGui::SliderFloat("Specular Power", &c.specularPower, 0.0f, 1000.0f, "%f", 5.0f);
				ImGui::SliderFloat("Specular Intensity", &c.specularIntensity, 0.0f, 2.0f);

				c.normalMapEnabled = normalMapEnabled ? TRUE : FALSE;

				pcb->Update(gfx, c);
				return true;
			}
		}
		else if constexpr (std::is_same<T, PSMaterialConstant_Diff>::value) {
			if (auto pcb = meshPtrs.front()->QueryBindable<Bind::PixelConstantBuffer<T>>()) {
				ImGui::Text("Material");

				ImGui::SliderFloat("Specular Power", &c.specularPower, 0.0f, 1000.0f, "%f", 5.0f);
				ImGui::SliderFloat("Specular Intensity", &c.specularIntensity, 0.0f, 2.0f);

				pcb->Update(gfx, c);
				return true;
			}
		}
		else if constexpr (std::is_same<T, PSMaterialConstant_Notex>::value) {
			if (auto pcb = meshPtrs.front()->QueryBindable<Bind::PixelConstantBuffer<T>>()) {
				ImGui::Text("Material");

				ImGui::SliderFloat("Specular Power", &c.specularPower, 0.0f, 1000.0f, "%f", 5.0f);

				ImGui::ColorPicker3("Specular Color", reinterpret_cast<float*>(&c.specularColor));
				ImGui::ColorPicker3("Diffuse Color", reinterpret_cast<float*>(&c.materialColor));

				pcb->Update(gfx, c);
				return true;
			}
		}
		return false;
	}


private:
	//Add a child to a node //this is private as models only want to be able to add a child 
	void AddChild(std::unique_ptr<Node> pChild) noxnd;

private:
	std::string name;
	int id;
	std::vector<std::unique_ptr<Node>> childPtrs; //a node can have zero or more children
	std::vector<Mesh*> meshPtrs; //a node can have zero or more meshes attached to it
	DirectX::XMFLOAT4X4 transform; //This represents the transform relative to the root node
	DirectX::XMFLOAT4X4 appliedTransform; //This represents additional transform on this part of the scene graph
};

class Model {
public:
	Model(Graphics& gfx, const std::string& pathString, float scale = 1.0f);

	void Draw(Graphics& gfx) const noxnd;
	void ShowWindow(Graphics& gfx, const char* windowName = nullptr) noexcept;

	void SetRootTransform(DirectX::FXMMATRIX tf) noexcept;

	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, const float scale);

	~Model() noexcept;

	std::unique_ptr<Node> ParseNode(int& nextId, const aiNode& node) noexcept;
private:
	static void BindVBuf(Graphics& gfx, std::vector<std::shared_ptr<Bindable>>& bindablePtrs, const std::string& meshTag, rsexp::VertexBuffer& vbuf, ID3DBlob& pvsbc);
	std::unique_ptr<Node> pRoot;
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
	std::unique_ptr<class ModelWindow> pWindow;


};