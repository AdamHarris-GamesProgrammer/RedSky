#pragma once
#include "Graphics.h"
#include "DynamicConstant.h"
#include <optional>
#include <unordered_map>

#include "Node.h"
#include "Mesh.h"

class Node;

class ModelWindow {
public:
	void Show(Graphics& gfx, const char* windowName, const Node& root) noexcept;

	void ApplyParamaters() noxnd;

private:
	DirectX::XMMATRIX GetTransform() const noxnd;

	const Dcb::Buffer& GetMaterial() const noxnd;

	bool TransformChanged() const noxnd {
		return pSelectedNode && transforms.at(pSelectedNode->GetID()).transformParamsTest;
	}
	void ResetTransformsChanged() noxnd {
		transforms.at(pSelectedNode->GetID()).transformParamsTest = false;
	}

	bool MaterialChanged() const noxnd {
		return pSelectedNode && transforms.at(pSelectedNode->GetID()).materialCbufTest;
	}
	void ResetMaterialsChanged() noxnd {
		transforms.at(pSelectedNode->GetID()).materialCbufTest = false;
	}
	bool IsChanged() const noxnd {
		return TransformChanged() || MaterialChanged();
	}

	Node* GetSelectedNode() const noexcept {
		return pSelectedNode;
	}
private:
	Node* pSelectedNode;

	struct TransformParameters
	{
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};
	struct NodeData
	{
		TransformParameters transformParams;
		bool transformParamsTest;
		std::optional<Dcb::Buffer> materialCbuf;
		bool materialCbufTest;
	};
	std::unordered_map<int, NodeData> transforms;
};