#include "Mesh.h"
#include "imgui/imgui.h"

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
	DirectX::XMStoreFloat4x4(&this->transform, transform);
}




void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransforms) const noxnd
{
	const auto built = DirectX::XMLoadFloat4x4(&transform) * accumulatedTransforms; //the current nodes transform plus the transform of all root objects
	for (const auto pm : meshPtrs) {
		pm->Draw(gfx, built);
	}
	for (const auto& pc : childPtrs) {
		pc->Draw(gfx, built);
	}
}

void Node::AddChild(std::unique_ptr<Node> pChild) noxnd {
	assert(pChild);
	childPtrs.push_back(std::move(pChild));
}

void Node::RenderTree() const noexcept
{
	if (ImGui::TreeNode(name.c_str())) {
		for (const auto& pChild : childPtrs) {
			pChild->RenderTree();
		}
		ImGui::TreePop();
	}
}

Model::Model(Graphics& gfx, const std::string fileName) {
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

void Model::Draw(Graphics& gfx) const noxnd {
	const auto transform = DirectX::XMMatrixRotationRollPitchYaw(pos.roll, pos.pitch, pos.yaw) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	pRoot->Draw(gfx, transform);
}

void Model::ShowWindow(const char* windowName /*= nullptr*/) noexcept
{
	windowName = windowName ? windowName : "Model";
	if (ImGui::Begin(windowName)) {
		ImGui::Columns(2, nullptr, true);
		pRoot->RenderTree();

		ImGui::NextColumn();
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll", &pos.roll, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", &pos.pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &pos.yaw, -180.0f, 180.0f);

		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos.x, -20.0f, 20.0f);
		ImGui::SliderFloat("Y", &pos.y, -20.0f, 20.0f);
		ImGui::SliderFloat("Z", &pos.z, -20.0f, 20.0f);
	}
	ImGui::End();
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
