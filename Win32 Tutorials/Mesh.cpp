#include "Mesh.h"
#include "imgui/imgui.h"
#include "Surface.h"
#include <unordered_map>
#include <sstream>
#include <filesystem>
#include "RedSkyXM.h"
#include "DynamicConstant.h"
#include "ConstantBufferEx.h"
#include "LayoutCodex.h"

namespace dx = DirectX;

Mesh::Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bindable>> bindPtrs) {
	AddBind(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	for (auto& pb : bindPtrs) {
		AddBind(std::move(pb));
	}

	AddBind(std::make_shared<TransformCbuf>(gfx, *this)); //Each mesh has a unique transform buffer
}

void Mesh::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd {
	DirectX::XMStoreFloat4x4(&transform, accumulatedTransform);
	Drawable::Draw(gfx);
}


Node::Node(int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in) noxnd
	: id(id), meshPtrs(meshPtrs), name(name)
{
	dx::XMStoreFloat4x4(&transform, transform_in);
	dx::XMStoreFloat4x4(&appliedTransform, dx::XMMatrixIdentity());
}

void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransforms) const noxnd
{
	const auto built =

		DirectX::XMLoadFloat4x4(&transform) *
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

const DirectX::XMFLOAT4X4& Node::GetAppliedTransform() const noexcept
{
	return appliedTransform;
}

void Node::AddChild(std::unique_ptr<Node> pChild) noxnd {
	assert(pChild);
	childPtrs.push_back(std::move(pChild));
}

void Node::ShowTree(Node*& pSelectedNode) const noexcept
{
	const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetID();
	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
		| ((GetID() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
		| ((childPtrs.empty()) ? ImGuiTreeNodeFlags_Leaf : 0);

	//generates the widget
	const auto expanded = ImGui::TreeNodeEx((void*)(intptr_t)GetID(), node_flags, name.c_str());


	if (ImGui::IsItemClicked()) {
		pSelectedNode = const_cast<Node*>(this);
	}

	if (expanded) {
		for (const auto& pChild : childPtrs) {
			pChild->ShowTree(pSelectedNode);
		}
		ImGui::TreePop();
	}

}

const Dcb::Buffer* Node::GetMaterialConstants() const noxnd
{
	if (meshPtrs.size() == 0)
	{
		return nullptr;
	}
	auto pBindable = meshPtrs.front()->QueryBindable<Bind::CachingPixelConstantBufferEX>();
	return &pBindable->GetBuffer();
}

void Node::SetMaterialConstants(const Dcb::Buffer& buf_in) noxnd
{
	auto pcb = meshPtrs.front()->QueryBindable<Bind::CachingPixelConstantBufferEX>();
	assert(pcb != nullptr);
	pcb->SetBuffer(buf_in);
}

class ModelWindow {
public:
	void Show(Graphics& gfx, const char* windowName, const Node& root) noexcept
	{
		windowName = windowName ? windowName : "Model";

		int nodeIndexTracker = 0;

		using namespace std::string_literals;
		if (ImGui::Begin(windowName)) {
			root.ShowTree(pSelectedNode);

			ImGui::Begin("Inspector");

			if (pSelectedNode != nullptr)
			{ //if there is a selected node


				ImGui::Text(pSelectedNode->GetName().c_str());

				const auto id = pSelectedNode->GetID();
				auto i = transforms.find(id);
				if (i == transforms.end())
				{
					const auto& applied = pSelectedNode->GetAppliedTransform();
					const auto angles = ExtractEulerAngles(applied);
					const auto translation = ExtractTranslation(applied);
					TransformParameters tp;
					tp.roll = angles.z;
					tp.pitch = angles.x;
					tp.yaw = angles.y;
					tp.x = translation.x;
					tp.y = translation.y;
					tp.z = translation.z;
					auto pMatConst = pSelectedNode->GetMaterialConstants();
					auto buf = pMatConst != nullptr ? std::optional<Dcb::Buffer>{*pMatConst} : std::optional<Dcb::Buffer>{};
					std::tie(i, std::ignore) = transforms.insert({ id,{ tp,false, std::move(buf),false } });
				}

				{
					auto& transform = i->second.transformParams;

					auto& changed = i->second.transformParamsTest;
					const auto changedCheck = [&changed](bool changed_in) { changed = changed || changed_in; };

					ImGui::Text("Orientation");
					changedCheck(ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f));
					changedCheck(ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f));
					changedCheck(ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f));

					ImGui::Text("Position");
					changedCheck(ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f));
					changedCheck(ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f));
					changedCheck(ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f));
				}
			

				if (i->second.materialCbuf) {
					auto& mat = *i->second.materialCbuf;

					auto& changed = i->second.materialCbufTest;
					const auto changedCheck = [&changed](bool changed_in) { changed = changed || changed_in; };

					ImGui::Text("Material");
					if (auto v = mat["normalMapEnabled"]; v.Exists())
					{
						changedCheck(ImGui::Checkbox("Norm Map", &v));
					}
					if (auto v = mat["specularMapEnabled"]; v.Exists())
					{
						changedCheck(ImGui::Checkbox("Spec Map", &v));
					}
					if (auto v = mat["hasGlossMap"]; v.Exists())
					{
						changedCheck(ImGui::Checkbox("Gloss Map", &v));
					}
					if (auto v = mat["materialColor"]; v.Exists())
					{
						changedCheck(ImGui::ColorPicker3("Diff Color", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
					}
					if (auto v = mat["specularPower"]; v.Exists())
					{
						changedCheck(ImGui::SliderFloat("Spec Power", &v, 0.0f, 100.0f, "%.1f", 1.5f));
					}
					if (auto v = mat["specularColor"]; v.Exists())
					{
						changedCheck(ImGui::ColorPicker3("Spec Color", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
					}
					if (auto v = mat["specularMapWeight"]; v.Exists())
					{
						changedCheck(ImGui::SliderFloat("Spec Weight", &v, 0.0f, 4.0f));
					}
					if (auto v = mat["specularIntensity"]; v.Exists())
					{
						changedCheck(ImGui::SliderFloat("Spec Intens", &v, 0.0f, 1.0f));
					}
				}
			}
			ImGui::End();
		}
		ImGui::End();
	}

	void ApplyParamaters() noxnd {
		if (TransformChanged()) {
			pSelectedNode->SetAppliedTransform(GetTransform());
			ResetTransformsChanged();
		}
		if (MaterialChanged()) {
			pSelectedNode->SetMaterialConstants(GetMaterial());
			ResetMaterialsChanged();
		}
	}

private:
	dx::XMMATRIX GetTransform() const noxnd {
		assert(pSelectedNode != nullptr);
		const auto& transform = transforms.at(pSelectedNode->GetID()).transformParams;
		return
			dx::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
			dx::XMMatrixTranslation(transform.x, transform.y, transform.z);
	}

	const Dcb::Buffer& GetMaterial() const noxnd {
		assert(pSelectedNode != nullptr);
		const auto& mat = transforms.at(pSelectedNode->GetID()).materialCbuf;
		assert(mat);
		return *mat;
	}

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
	//TODO: Add support for editing multiple materials on each mesh
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

Model::Model(Graphics& gfx, const std::string& pathString, const float scale) :
	pWindow(std::make_unique<ModelWindow>()) {
	//Create Assimp object and load file
	Assimp::Importer imp;
	const auto pScene = imp.ReadFile(pathString.c_str(),
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace
	);

	if (pScene == nullptr) {
		throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
	}

	//load all meshes and store them 
	for (size_t i = 0; i < pScene->mNumMeshes; i++) {
		meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials, pathString, scale));
	}

	int nextId = 0;
	//pointer to the root node of the scene
	pRoot = ParseNode(nextId, *pScene->mRootNode);
}

Model::~Model() noexcept {}

void Model::Draw(Graphics& gfx) const noxnd {
	pWindow->ApplyParamaters();

	pRoot->Draw(gfx, dx::XMMatrixIdentity());
}

void Model::ShowWindow(Graphics& gfx, const char* windowName) noexcept {
	pWindow->Show(gfx, windowName, *pRoot);
}

void Model::SetRootTransform(DirectX::FXMMATRIX tf) noexcept
{
	pRoot->SetAppliedTransform(tf);
}

std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, const float scale) {
	using namespace std::string_literals;
	using rsexp::VertexLayout;
	using namespace Bind;

	std::vector<std::shared_ptr<Bindable>> bindablePtrs;

	const auto rootPath = path.parent_path().string() + "\\";

	bool hasSpecularMap = false;
	bool hasAlphaGloss = false;
	bool hasNormalMap = false;
	bool hasDiffuseMap = false;
	bool hasAlphaDiffuse = false;
	float shininess = 2.0f;
	dx::XMFLOAT4 specularColor = { 0.18f,0.18f,0.18f,1.0f };
	dx::XMFLOAT4 diffuseColor = { 0.45f,0.45f,0.45f,1.0f };

	if (mesh.mMaterialIndex >= 0) {
		auto& material = *pMaterials[mesh.mMaterialIndex];

		aiString texFileName;

		if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS) {
			auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str());
			hasAlphaDiffuse = tex->HasAlpha();
			bindablePtrs.push_back(std::move(tex));
			hasDiffuseMap = true;
		}
		else
		{
			material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
		}

		if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS) {
			auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 1);
			hasAlphaGloss = tex->HasAlpha();
			bindablePtrs.push_back(std::move(tex));
			hasAlphaGloss = true;
			hasSpecularMap = true;
		}
		else
		{
			material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(specularColor));
		}
		if (!hasAlphaGloss)
		{
			material.Get(AI_MATKEY_SHININESS, shininess);
		}

		if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS) {
			auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 2);
			hasAlphaGloss = tex->HasAlpha();
			bindablePtrs.push_back(std::move(tex));
			hasNormalMap = true;
		}

		if (hasDiffuseMap || hasNormalMap || hasSpecularMap) {
			bindablePtrs.push_back(Sampler::Resolve(gfx));
		}
	}

	auto meshTag = rootPath + "%" + mesh.mName.C_Str();

	std::vector<unsigned short> indices;
	indices.reserve(mesh.mNumFaces * 3);
	for (unsigned int i = 0; i < mesh.mNumFaces; i++)
	{
		const auto& face = mesh.mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

	std::string vsShader;
	std::string psShader;

	if (hasDiffuseMap && hasNormalMap && hasSpecularMap) {
		vsShader = "PhongVSNormalMap.cso";
		psShader = hasAlphaDiffuse ? "PhongPSSpecNormalMask.cso" : "PhongPSSpecNormalMap.cso";
	}
	else if (hasDiffuseMap && !hasNormalMap && hasSpecularMap) {
		vsShader = "PhongVS.cso";
		psShader = "PhongPSSpec.cso";
	}
	else if (hasDiffuseMap && hasNormalMap) {
		vsShader = "PhongVSNormalMap.cso";
		psShader = "PhongPSNormalMap.cso";
	}
	else if (hasDiffuseMap) {
		vsShader = "PhongVS.cso";
		psShader = "PhongPS.cso";
	}
	else if (!hasDiffuseMap && !hasNormalMap && !hasSpecularMap) {
		vsShader = "PhongVSNotex.cso";
		psShader = "PhongPSNotex.cso";
	}
	else
	{
		throw std::runtime_error("Invalid Material Combination");
	}

	auto pvs = VertexShader::Resolve(gfx, vsShader);
	auto pvsbc = pvs->GetByteCode();
	bindablePtrs.push_back(std::move(pvs));

	bindablePtrs.push_back(PixelShader::Resolve(gfx, psShader));

	if (hasDiffuseMap && hasNormalMap && hasSpecularMap) {
		rsexp::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Tangent)
			.Append(VertexLayout::Bitangent)
			.Append(VertexLayout::Texture2D)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		BindVBuf(gfx, bindablePtrs, meshTag, vbuf, *pvsbc);
		
		Dcb::RawLayout lay;
		lay.Add<Dcb::Bool>("normalMapEnabled");
		lay.Add<Dcb::Bool>("specularMapEnabled");
		lay.Add<Dcb::Bool>("hasGloss");
		lay.Add<Dcb::Float>("specularPower");
		lay.Add<Dcb::Float3>("specularColor");
		lay.Add<Dcb::Float>("specularMapWeight");

		auto buf = Dcb::Buffer(std::move(lay));
		buf["normalMapEnabled"] = true;
		buf["specularMapEnabled"] = true;
		buf["hasGloss"] = hasAlphaGloss;
		buf["specularPower"] = shininess;
		buf["specularColor"] = dx::XMFLOAT3{ 0.75f,0.75f,0.75f };
		buf["specularMapWeight"] = 0.671f;

		bindablePtrs.push_back(std::make_shared<CachingPixelConstantBufferEX>(gfx, buf, 1u));

	}
	else if (hasDiffuseMap && !hasNormalMap && hasSpecularMap)
	{
		rsexp::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Texture2D)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		BindVBuf(gfx, bindablePtrs, meshTag, vbuf, *pvsbc);

		Dcb::RawLayout lay;
		lay.Add<Dcb::Float>("specularPower");
		lay.Add<Dcb::Bool>("hasGloss");
		lay.Add<Dcb::Float>("specularMapWeight");

		auto buf = Dcb::Buffer(std::move(lay));
		buf["specularPower"] = shininess;
		buf["hasGloss"] = hasAlphaGloss;
		buf["specularMapWeight"] = 1.0f;

		bindablePtrs.push_back(std::make_unique<Bind::CachingPixelConstantBufferEX>(gfx, buf, 1u));

	}
	else if (hasDiffuseMap && hasNormalMap)
	{
		rsexp::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Tangent)
			.Append(VertexLayout::Bitangent)
			.Append(VertexLayout::Texture2D)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		BindVBuf(gfx, bindablePtrs, meshTag, vbuf, *pvsbc);

		Dcb::RawLayout layout;

		layout.Add<Dcb::Float>("specularIntensity");
		layout.Add<Dcb::Float>("specularPower");
		layout.Add<Dcb::Bool>("normalMapEnabled");

		auto cbuf = Dcb::Buffer(std::move(layout));
		cbuf["specularIntensity"] = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
		cbuf["specularPower"] = shininess;
		cbuf["normalMapEnabled"] = true;
		bindablePtrs.push_back(std::make_shared<CachingPixelConstantBufferEX>(gfx, cbuf, 1u));

	}
	else if (hasDiffuseMap) {
		rsexp::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Texture2D)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		BindVBuf(gfx, bindablePtrs, meshTag, vbuf, *pvsbc);

		Dcb::RawLayout lay;
		lay.Add<Dcb::Float>("specularPower");
		lay.Add<Dcb::Float>("specularIntensity");
		lay.Add<Dcb::Float>("specularMapWeight");

		auto buf = Dcb::Buffer(std::move(lay));
		buf["specularPower"] = shininess;
		buf["specularIntensity"] = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
		buf["specularMapWeight"] = 1.0f;

		bindablePtrs.push_back(std::make_unique<Bind::CachingPixelConstantBufferEX>(gfx, buf, 1u));

	}
	else if (!hasDiffuseMap && !hasNormalMap && !hasSpecularMap)
	{
		rsexp::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
			);
		}

		BindVBuf(gfx, bindablePtrs, meshTag, vbuf, *pvsbc);

		Dcb::RawLayout lay;
		lay.Add<Dcb::Float4>("materialColor");
		lay.Add<Dcb::Float4>("specularColor");
		lay.Add<Dcb::Float>("specularPower");

		auto buf = Dcb::Buffer(std::move(lay));
		buf["specularPower"] = shininess;
		buf["specularColor"] = specularColor;
		buf["materialColor"] = diffuseColor;

		bindablePtrs.push_back(std::make_unique<Bind::CachingPixelConstantBufferEX>(gfx, buf, 1u));
	}


	//anything with a alpha diffuse will be a two sided object
	bindablePtrs.push_back(Rasterizer::Resolve(gfx, hasAlphaDiffuse));

	bindablePtrs.push_back(Blender::Resolve(gfx, false));

	//Returns the vector of mesh bindables
	return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
}

std::unique_ptr<Node> Model::ParseNode(int& nextId, const aiNode& node) noexcept {
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

	auto pNode = std::make_unique<Node>(nextId++, node.mName.C_Str(), std::move(curMeshPtrs), transform);
	for (size_t i = 0; i < node.mNumChildren; i++) {
		pNode->AddChild(ParseNode(nextId, *node.mChildren[i]));
	}

	return pNode;
}

void Model::BindVBuf(Graphics& gfx, std::vector<std::shared_ptr<Bindable>>& bindablePtrs, const std::string& meshTag, rsexp::VertexBuffer& vbuf, ID3DBlob& pvsbc)
{
	bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));
	bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), &pvsbc));
}

#pragma region 
ModelException::ModelException(int line, const char* file, std::string note) noexcept
	: RedSkyException(line, file), note(std::move(note)) {}

const char* ModelException::what() const noexcept
{
	std::ostringstream oss;
	oss << RedSkyException::what() << std::endl
		<< "[Note]: " << GetNote();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* ModelException::GetType() const noexcept
{
	return "RedSky Model Exception";
}

const std::string& ModelException::GetNote() const noexcept
{
	return note;
}
#pragma endregion Model Exception Class