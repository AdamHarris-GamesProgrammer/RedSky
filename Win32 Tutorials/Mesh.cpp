#include "Mesh.h"
#include "imgui/imgui.h"
#include <unordered_map>

namespace dx = DirectX;

Mesh::Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs) {
	if (!IsStaticInitialised()) {
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	for (auto& pb : bindPtrs) {
		//if it is a index buffer than calls the special method
		if (auto pi = dynamic_cast<IndexBuffer*>(pb.get())) {
			AddIndexBuffer(std::unique_ptr<IndexBuffer>{pi});
			pb.release(); //release the index buffer, only want one unique pointer at a given time
		}
		else
		{
			AddBind(std::move(pb));
		}
	}

	AddBind(std::make_unique<TransformCbuf>(gfx, *this)); //Each mesh has a unique transform buffer
}

void Mesh::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd {
	DirectX::XMStoreFloat4x4(&transform, accumulatedTransform);
	Drawable::Draw(gfx);
}


Node::Node(const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd 
	: meshPtrs(meshPtrs), name(name)
{
	dx::XMStoreFloat4x4(&baseTransform, transform);
	dx::XMStoreFloat4x4(&appliedTransform, dx::XMMatrixIdentity());
}

void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransforms) const noxnd
{
	const auto built = 
		DirectX::XMLoadFloat4x4(&baseTransform) *
		dx::XMLoadFloat4x4(&appliedTransform) *
		accumulatedTransforms; //the current nodes transform plus the transform of all root objects
	for (const auto pm : meshPtrs) {
		pm->Draw(gfx, built);
	}
	for (const auto& pc : childPtrs) {
		pc->Draw(gfx, built);
	}
}

void Node::SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept
{
	dx::XMStoreFloat4x4(&appliedTransform, transform);
}

void Node::AddChild(std::unique_ptr<Node> pChild) noxnd {
	assert(pChild);
	childPtrs.push_back(std::move(pChild));
}

void Node::ShowTree(int& nodeIndex, std::optional<int>& selectedIndex, Node*& pSelectedNode) const noexcept
{
	//gets the current node 
	const int currentNodeIndex = nodeIndex;
	nodeIndex++; //increments the current node for recursive calling

	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow 
		| ((currentNodeIndex == selectedIndex.value_or(-1)) ? ImGuiTreeNodeFlags_Selected : 0) 
		| ((childPtrs.empty()) ? ImGuiTreeNodeFlags_Leaf : 0);

	if (ImGui::TreeNodeEx((void*)(intptr_t)currentNodeIndex, node_flags, name.c_str())) {
		if (ImGui::IsItemClicked()) {
			selectedIndex = currentNodeIndex;
			pSelectedNode = const_cast<Node*>(this);
		}
		
		for (const auto& pChild : childPtrs) {
			pChild->ShowTree(nodeIndex, selectedIndex, pSelectedNode);
		}
		ImGui::TreePop();
	}

}

class ModelWindow {
public:
	void Show(const char* windowName, const Node& root) noexcept
	{
		windowName = windowName ? windowName : "Model";

		int nodeIndexTracker = 0;

		if (ImGui::Begin(windowName)) {
			ImGui::Columns(2, nullptr, true);
			root.ShowTree(nodeIndexTracker, selectedIndex, pSelectedNode);

			ImGui::NextColumn();
			if (pSelectedNode != nullptr) {
				auto& transform = transforms[*selectedIndex];
				ImGui::Text("Orientation");
				ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
				ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
				ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);

				ImGui::Text("Position");
				ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f);
				ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f);
				ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f);
			}

		}
		ImGui::End();
	}

	dx::XMMATRIX GetTransform() const noexcept {
		const auto& transform = transforms.at(*selectedIndex);
		return 
			dx::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
			dx::XMMatrixTranslation(transform.x, transform.y, transform.z);
	}

	Node* GetSelectedNode() const noexcept {
		return pSelectedNode;
	}

private:
	std::optional<int> selectedIndex;
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
	std::unordered_map<int, TransformParameters> transforms;
};



Model::Model(Graphics& gfx, const std::string fileName) :
	pWindow(std::make_unique<ModelWindow>()){
	//Create Assimp object and load file
	Assimp::Importer imp;
	const auto pScene = imp.ReadFile(fileName.c_str(),
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate
		);

	//load all meshes and store them 
	for (size_t i = 0; i < pScene->mNumMeshes; i++) {
		meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));
	}

	//pointer to the root node of the scene
	pRoot = ParseNode(*pScene->mRootNode);
}

Model::~Model() noexcept {}

void Model::Draw(Graphics& gfx) const noxnd {
	if (auto node = pWindow->GetSelectedNode()) {
		node->SetAppliedTransform(pWindow->GetTransform());
	}
	pRoot->Draw(gfx, dx::XMMatrixIdentity());
}

void Model::ShowWindow(const char* windowName) noexcept {
	pWindow->Show(windowName, *pRoot);
}

std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh) {
	namespace dx = DirectX;

	using rsexp::VertexLayout;

	//create a dynamic buffer layout 
	rsexp::VertexBuffer vbuf(std::move(
		VertexLayout{}
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
		));

	for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
		//places the vertices and normals in the vertex buffer
		vbuf.EmplaceBack(
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
			);
	}

	//loop through the faces and define the indices
	std::vector<unsigned short> indices;
	indices.reserve(mesh.mNumFaces * 3);
	for (unsigned int i = 0; i < mesh.mNumFaces; i++) {
		const auto& face = mesh.mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	//vector object to hold all of the bindable objects
	std::vector<std::unique_ptr<Bindable>> bindablePtrs;

	//create vertex buffer
	bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, vbuf));

	//create index buffer
	bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices));


	//Create shader buffers
	auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
	auto pvsbc = pvs->GetByteCode();
	bindablePtrs.push_back(std::move(pvs));

	bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

	//Create input layout bindable
	bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

	struct PSMaterialConstant {
		DirectX::XMFLOAT3 color = { 0.6f,0.6f,0.8f };
		float specularIntensity = 0.6f;
		float specularPower = 30.0f;
		float padding[3];
	} pmc;

	bindablePtrs.push_back(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));

	//Returns the vector of mesh bindables
	return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
}

std::unique_ptr<Node> Model::ParseNode(const aiNode& node) noexcept {
	namespace dx = DirectX;
	//converts to column major for gpu processing
	const auto transform = dx::XMMatrixTranspose(dx::XMLoadFloat4x4(
		reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
		));

	//array of meshes
	std::vector<Mesh*> curMeshPtrs;
	curMeshPtrs.reserve(node.mNumMeshes);
	for (size_t i = 0; i < node.mNumMeshes; i++) {
		const auto meshIdx = node.mMeshes[i];
		curMeshPtrs.push_back(meshPtrs.at(meshIdx).get());
	}

	auto pNode = std::make_unique<Node>(node.mName.C_Str(), std::move(curMeshPtrs), transform);
	for (size_t i = 0; i < node.mNumChildren; i++) {
		pNode->AddChild(ParseNode(*node.mChildren[i]));
	}

	return pNode;
}
