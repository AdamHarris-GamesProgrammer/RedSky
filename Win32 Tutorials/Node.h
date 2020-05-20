#pragma once
#include "Graphics.h"

class Model;
class Mesh;
class FrameCommander;

class Node {
	friend Model;
public:
	Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in) noxnd;

	int GetID() const noexcept { return id; }
	void Submit(FrameCommander& frame, DirectX::FXMMATRIX accumulatedTransforms) const noxnd;
	void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
	const DirectX::XMFLOAT4X4& GetAppliedTransform() const noexcept;
	//void ShowTree(Node*& pSelectedNode) const noexcept;
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