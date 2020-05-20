#include "Node.h"
#include "Mesh.h"
#include "imgui/imgui.h"

namespace dx = DirectX;


Node::Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in) noxnd
	: id(id), meshPtrs(meshPtrs), name(name)
{
	dx::XMStoreFloat4x4(&transform, transform_in);
	dx::XMStoreFloat4x4(&appliedTransform, dx::XMMatrixIdentity());
}

void Node::Submit(FrameCommander& frame, DirectX::FXMMATRIX accumulatedTransforms) const noxnd
{
	const auto built =
		DirectX::XMLoadFloat4x4(&transform) *
		dx::XMLoadFloat4x4(&appliedTransform) *
		accumulatedTransforms; //the current nodes transform plus the transform of all root objects
	for (const auto pm : meshPtrs) {
		pm->Submit(frame, accumulatedTransforms);
	}
	for (const auto& pc : childPtrs) {
		pc->Submit(frame, accumulatedTransforms);
	}
}

void Node::SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept
{
	dx::XMStoreFloat4x4(&appliedTransform, transform);
}

const DirectX::XMFLOAT4X4& Node::GetAppliedTransform() const noexcept
{
	return appliedTransform;
}

void Node::AddChild(std::unique_ptr<Node> pChild) noxnd {
	assert(pChild);
	childPtrs.push_back(std::move(pChild));
}

//void Node::ShowTree(Node*& pSelectedNode) const noexcept
//{
//	const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetID();
//	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
//		| ((GetID() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
//		| ((childPtrs.empty()) ? ImGuiTreeNodeFlags_Leaf : 0);
//
//	//generates the widget
//	const auto expanded = ImGui::TreeNodeEx((void*)(intptr_t)GetID(), node_flags, name.c_str());
//
//
//	if (ImGui::IsItemClicked()) {
//		pSelectedNode = const_cast<Node*>(this);
//	}
//
//	if (expanded) {
//		for (const auto& pChild : childPtrs) {
//			pChild->ShowTree(pSelectedNode);
//		}
//		ImGui::TreePop();
//	}
//
//}
