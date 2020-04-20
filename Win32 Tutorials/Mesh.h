#pragma once
#include "DrawableBase.h"
#include "BindableCommon.h"
#include "Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Bind;

//Mesh is a drawable object
class Mesh : public DrawableBase<Mesh> {
public:
	//takes in a vector of bind ables //alllows the user to pass in what bindables should comprise the mesh
	Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs);

	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
	DirectX::XMMATRIX GetTransformXM() const noexcept override { return DirectX::XMLoadFloat4x4(&transform); } //This is called in the Drawable::Draw method

private:
	mutable DirectX::XMFLOAT4X4 transform;
};

class Node {
	friend class Model;
public:
	Node(std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd : meshPtrs(meshPtrs) {
		DirectX::XMStoreFloat4x4(&this->transform, transform);
	}

	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransforms) const noxnd;

private:
	//Add a child to a node //this is private as models only want to be able to add a child 
	void AddChild(std::unique_ptr<Node> pChild) noxnd;

private:
	std::vector<std::unique_ptr<Node>> childPtrs; //a node can have zero or more children
	std::vector<Mesh*> meshPtrs; //a node can have zero or more meshes attached to it
	DirectX::XMFLOAT4X4 transform; //This represents the transform relative to the root node
};

class Model {
public:
	Model(Graphics& gfx, const std::string fileName);

	void Draw(Graphics& gfx, DirectX::FXMMATRIX transform) const;

	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh);

	std::unique_ptr<Node> ParseNode(const aiNode& node);

	void Draw(Graphics& gfx);

private:
	std::unique_ptr<Node> pRoot;
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
};