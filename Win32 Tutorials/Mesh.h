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
	Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in) noxnd;

	int GetID() const noexcept { return id; }
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransforms) const noxnd;
	void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
	const DirectX::XMFLOAT4X4& GetAppliedTransform() const noexcept;
	void ShowTree(Node*& pSelectedNode) const noexcept;
	std::string GetName() const noexcept { return name; }

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