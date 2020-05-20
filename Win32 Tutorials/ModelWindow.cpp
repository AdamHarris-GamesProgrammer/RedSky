#include "ModelWindow.h"

namespace dx = DirectX;

void ModelWindow::Show(Graphics& gfx, const char* windowName, const Node& root) noexcept
{
	//windowName = windowName ? windowName : "Model";

	//int nodeIndexTracker = 0;

	//using namespace std::string_literals;
	//if (ImGui::Begin(windowName)) {
	//	root.ShowTree(pSelectedNode);

	//	ImGui::Begin("Inspector");

	//	if (pSelectedNode != nullptr)
	//	{ //if there is a selected node


	//		ImGui::Text(pSelectedNode->GetName().c_str());

	//		const auto id = pSelectedNode->GetID();
	//		auto i = transforms.find(id);
	//		if (i == transforms.end())
	//		{
	//			const auto& applied = pSelectedNode->GetAppliedTransform();
	//			const auto angles = ExtractEulerAngles(applied);
	//			const auto translation = ExtractTranslation(applied);
	//			TransformParameters tp;
	//			tp.roll = angles.z;
	//			tp.pitch = angles.x;
	//			tp.yaw = angles.y;
	//			tp.x = translation.x;
	//			tp.y = translation.y;
	//			tp.z = translation.z;
	//			auto pMatConst = pSelectedNode->GetMaterialConstants();
	//			auto buf = pMatConst != nullptr ? std::optional<Dcb::Buffer>{*pMatConst} : std::optional<Dcb::Buffer>{};
	//			std::tie(i, std::ignore) = transforms.insert({ id,{ tp,false, std::move(buf),false } });
	//		}

	//		{
	//			auto& transform = i->second.transformParams;

	//			auto& changed = i->second.transformParamsTest;
	//			const auto changedCheck = [&changed](bool changed_in) { changed = changed || changed_in; };

	//			ImGui::Text("Orientation");
	//			changedCheck(ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f));
	//			changedCheck(ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f));
	//			changedCheck(ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f));

	//			ImGui::Text("Position");
	//			changedCheck(ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f));
	//			changedCheck(ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f));
	//			changedCheck(ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f));
	//		}
	//	

	//		if (i->second.materialCbuf) {
	//			auto& mat = *i->second.materialCbuf;

	//			auto& changed = i->second.materialCbufTest;
	//			const auto changedCheck = [&changed](bool changed_in) { changed = changed || changed_in; };

	//			ImGui::Text("Material");
	//			if (auto v = mat["normalMapEnabled"]; v.Exists())
	//			{
	//				changedCheck(ImGui::Checkbox("Norm Map", &v));
	//			}
	//			if (auto v = mat["specularMapEnabled"]; v.Exists())
	//			{
	//				changedCheck(ImGui::Checkbox("Spec Map", &v));
	//			}
	//			if (auto v = mat["hasGlossMap"]; v.Exists())
	//			{
	//				changedCheck(ImGui::Checkbox("Gloss Map", &v));
	//			}
	//			if (auto v = mat["materialColor"]; v.Exists())
	//			{
	//				changedCheck(ImGui::ColorPicker3("Diff Color", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
	//			}
	//			if (auto v = mat["specularPower"]; v.Exists())
	//			{
	//				changedCheck(ImGui::SliderFloat("Spec Power", &v, 0.0f, 100.0f, "%.1f", 1.5f));
	//			}
	//			if (auto v = mat["specularColor"]; v.Exists())
	//			{
	//				changedCheck(ImGui::ColorPicker3("Spec Color", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
	//			}
	//			if (auto v = mat["specularMapWeight"]; v.Exists())
	//			{
	//				changedCheck(ImGui::SliderFloat("Spec Weight", &v, 0.0f, 4.0f));
	//			}
	//			if (auto v = mat["specularIntensity"]; v.Exists())
	//			{
	//				changedCheck(ImGui::SliderFloat("Spec Intens", &v, 0.0f, 1.0f));
	//			}
	//		}
	//	}
	//	ImGui::End();
	//}
	//ImGui::End();
}

void ModelWindow::ApplyParamaters() noxnd
{
	//if (TransformChanged()) {
	//	pSelectedNode->SetAppliedTransform(GetTransform());
	//	ResetTransformsChanged();
	//}
	//if (MaterialChanged()) {
	//	pSelectedNode->SetMaterialConstants(GetMaterial());
	//	ResetMaterialsChanged();
	//}
}

DirectX::XMMATRIX ModelWindow::GetTransform() const noxnd
{
	assert(pSelectedNode != nullptr);
	const auto& transform = transforms.at(pSelectedNode->GetID()).transformParams;
	return
		dx::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
		dx::XMMatrixTranslation(transform.x, transform.y, transform.z);
}

const Dcb::Buffer& ModelWindow::GetMaterial() const noxnd
{
	assert(pSelectedNode != nullptr);
	const auto& mat = transforms.at(pSelectedNode->GetID()).materialCbuf;
	assert(mat);
	return *mat;
}




