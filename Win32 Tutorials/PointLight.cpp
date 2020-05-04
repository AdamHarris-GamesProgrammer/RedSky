#include "PointLight.h"
#include "imgui/imgui.h"

PointLight::PointLight(Graphics& gfx, float radius /*= 0.5f*/) : mesh(gfx, radius), cbuf(gfx) {
	Reset();
}

void PointLight::SpawnControlWindow() noexcept
{
	//Allows us to manipulate the colors of the light 
	if (ImGui::Begin("Light")) {
		ImGui::Text("Position");
		ImGui::SliderFloat3("", &cbData.pos.x, -60.0f, 60.0f, "%.1f");

		ImGui::Text("Intensity/Color");
		ImGui::SliderFloat("Intensity", &cbData.diffuseIntensity, 0.01f, 2.0f, "%.2f", 2);
		ImGui::ColorEdit3("Diffuse Color", &cbData.diffuseColor.x);
		ImGui::ColorEdit3("Ambient", &cbData.ambient.x);

		ImGui::Text("Falloff");
		ImGui::SliderFloat("Constant", &cbData.attConst, 0.05f, 10.0f, "%.2f", 4);
		ImGui::SliderFloat("Linear", &cbData.attLin, 0.0001f, 4.0f, "%.4f", 8);
		ImGui::SliderFloat("Quadratic", &cbData.attQuad, 0.0000001f, 10.0f, "%.7f", 10);
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}
	ImGui::End();
}

void PointLight::Reset() noexcept
{
	cbData = {
		{-14.5f,32.5f,0.0f},	//Position
		{0.15f,0.15f,0.15f},	//Ambient
		{1.0f,1.0f,1.0f},		//Diffuse 
		2.0f,					//Diffuse Intensity
		1.0f,					//Attenuation Constant
		0.045f,					//Attenuation Linear
		0.003f,				//Attenuation Quadratic
	};
}

void PointLight::Draw(Graphics& gfx) const noxnd
{
	mesh.SetPos(cbData.pos);
	mesh.Draw(gfx);
}

void PointLight::Bind(Graphics& gfx, DirectX::FXMMATRIX view) const noexcept
{
	auto dataCopy = cbData;
	const auto pos = DirectX::XMLoadFloat3(&cbData.pos);
	DirectX::XMStoreFloat3(&dataCopy.pos, DirectX::XMVector3Transform(pos, view));

	cbuf.Update(gfx, dataCopy);
	cbuf.Bind(gfx);
}
