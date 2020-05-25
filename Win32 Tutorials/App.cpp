#include "App.h"
#include <sstream>
#include <iomanip>
#include "Constants.h"
#include <memory>
#include "RedSkyMath.h"
#include <algorithm>
#include <random>
#include "Surface.h"
#include "imgui/imgui.h"
#include "Vertex.h"
#include "TexturePreprocessor.h"
#include <shellapi.h>
#include <dxtex/DirectXTex.h>
#include "RedSkyUtility.h"
#include "DynamicConstant.h"
#include "LayoutCodex.h"
#include "Testing.h"
#include "PerformanceLog.h"
#include "RedSkyKeyboardKeys.h"

#include "ModelProbe.h"
#include "Node.h"
#include "RedSkyXM.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "DynamicConstant.h"

namespace DX = DirectX;

App::App(const std::string& commandLine) :
	commandLine(commandLine),
	wnd(1280, 720, "RedSky Demo Window"),
	scriptCommander(TokenizeQuoted(commandLine)),
	light(wnd.Gfx())
{
	//TestMaterialSystemLoading(wnd.Gfx());
	//TestDynamicMeshLoading();
	//TestDynamicConstant();

	cube.SetPos({ 4.0f,0.0f,0.0f });
	cube2.SetPos({ 0.0f,4.0f,0.0 });

	wnd.Gfx().SetProjection(DX::XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 400.0f));
}

App::~App() {}

int App::Go()
{
	while (true)
	{
		if (const auto ecode = Window::ProcessMessages()) {
			return *ecode; //if it has a value then it means there is a WM_QUIT message.
		}
		DoFrame();
	}
}

void App::DoFrame()
{
	auto dt = timer.Mark() * simSpeed;

	wnd.Gfx().BeginFrame(bgColour);
	wnd.Gfx().SetCamera(cam.GetMatrix());
	light.Bind(wnd.Gfx(), cam.GetMatrix());

	PollInput(dt);

	while (const auto delta = wnd.mouse.ReadRawDelta())
	{
		if (!wnd.CursorEnabled()) {
			cam.Rotate((float)delta->x, (float)delta->y);
		}
	}


	light.Submit(fc);
	cube.Submit(fc);
	cube2.Submit(fc);

	//goblin.Submit(fc);
	sponza.Submit(fc);

	fc.Execute(wnd.Gfx());

	class TP : public TechniqueProbe {
	public:
		void OnSetTechnique() override {
			using namespace std::string_literals;
			ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, pTech->GetName().c_str());
			bool active = pTech->IsActive();
			ImGui::Checkbox(("Tech Active##"s + std::to_string(techIdx)).c_str(), &active);
			pTech->SetActiveState(active);
		}

		bool OnVisitBuffer(Dcb::Buffer& buf) override {
			namespace dx = DirectX;
			float test = false;
			const auto dcheck = [&test](bool changed) { test = test || changed; };
			auto tag = [tagScratch = std::string{}, tagString = "##" + std::to_string(bufIdx)]
			(const char* label) mutable{
				tagScratch = label + tagString;
				return tagScratch.c_str();
			};

			if (auto v = buf["scale"]; v.Exists()) {
				dcheck(ImGui::SliderFloat(tag("Scale"), &v, 1.0f, 2.0f, "%.3f", 3.5f));
			}
			if (auto v = buf["materialColor"]; v.Exists()) {
				dcheck(ImGui::ColorPicker3(tag("Color"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
			}
			if (auto v = buf["specularColor"]; v.Exists()) {
				dcheck(ImGui::ColorPicker3(tag("Spec. Color"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
			}
			if (auto v = buf["specularGloss"]; v.Exists()) {
				dcheck(ImGui::SliderFloat(tag("Glossiness"), &v, 1.0f, 100.0f, "%.1f", 1.5f));
			}
			if (auto v = buf["specularWeight"]; v.Exists()) {
				dcheck(ImGui::SliderFloat(tag("Spec. Weight"), &v, 0.0f, 2.0f));
			}
			if (auto v = buf["useNormalMap"]; v.Exists()) {
				dcheck(ImGui::Checkbox(tag("Normal Map Enabled"), &v));
			}
			if (auto v = buf["normalMapWeight"]; v.Exists()) {
				dcheck(ImGui::SliderFloat(tag("Normal Map Weight"), &v, 0.0f, 2.0f));
			}
			return test;
		}
	} probe;

	class MP : ModelProbe {
	public:
		void SpawnWindow(Model& model) {
			namespace dx = DirectX;

			ImGui::Begin("Model");
			ImGui::Columns(2, nullptr, true);
			model.Accept(*this);

			ImGui::NextColumn();
			if (pSelectedNode != nullptr) {
				bool test = false;
				const auto dcheck = [&test](bool changed) { test = test || changed; };
				auto& tf = ResolveTransform();
				ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, "Translation");
				dcheck(ImGui::SliderFloat("X", &tf.x, -60.0f, 60.0f));
				dcheck(ImGui::SliderFloat("Y", &tf.y, -60.0f, 60.0f));
				dcheck(ImGui::SliderFloat("Z", &tf.z, -60.0f, 60.0f));
				ImGui::TextColored({ 0.4f,1.0f,0.6f, 1.0f }, "Orientation");
				dcheck(ImGui::SliderAngle("X-Rotation", &tf.xRot, -180.0f, 180.0f));
				dcheck(ImGui::SliderAngle("Y-Rotation", &tf.yRot, -180.0f, 180.0f));
				dcheck(ImGui::SliderAngle("Z-Rotation", &tf.zRot, -180.0f, 180.0f));
				if (test) {
					pSelectedNode->SetAppliedTransform(
						dx::XMMatrixRotationX(tf.xRot) *
						dx::XMMatrixRotationX(tf.yRot) *
						dx::XMMatrixRotationX(tf.zRot) *
						dx::XMMatrixTranslation(tf.x, tf.y, tf.z)
					);
				}
			}
			ImGui::End();
		}


	protected:
		bool PushNode(Node& node) override {
			const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();

			const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
				| ((node.GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
				| (node.HasChildren() ? 0 : ImGuiTreeNodeFlags_Leaf);

			const auto expanded = ImGui::TreeNodeEx(
				(void*)(intptr_t)node.GetId(),
				node_flags, node.GetName().c_str()
			);

			if (ImGui::IsItemClicked()) {
				pSelectedNode = &node;
			}
			return expanded;
		}
		void PopNode(Node& node) override {
			ImGui::TreePop();
		}
	private:
		Node* pSelectedNode = nullptr;

		struct TransformParamaters {
			float xRot = 0.0f;
			float yRot = 0.0f;
			float zRot = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
		};
		std::unordered_map<int, TransformParamaters> transformParams;
	private:
		TransformParamaters& ResolveTransform() noexcept {
			const auto id = pSelectedNode->GetId();
			auto i = transformParams.find(id);
			if (i == transformParams.end()) {
				return LoadTransform(id);
			}
			return i->second;
		}
		TransformParamaters& LoadTransform(int id) noexcept {
			const auto& applied = pSelectedNode->GetAppliedTransform();
			const auto angles = ExtractEulerAngles(applied);
			const auto translation = ExtractTranslation(applied);
			TransformParamaters tp;
			tp.zRot = angles.z;
			tp.yRot = angles.y;
			tp.xRot = angles.x;
			tp.x = translation.x;
			tp.y = translation.y;
			tp.z = translation.z;
			return transformParams.insert({ id, {tp} }).first->second;
		}
	};
	static MP modelProbe;

	modelProbe.SpawnWindow(sponza);
	SpawnBackgroundControlWindow();
	cam.SpawnControlWindow();
	light.SpawnControlWindow();

	cube.SpawnControlWindow(wnd.Gfx(), "Cube 1");
	cube2.SpawnControlWindow(wnd.Gfx(), "Cube 2");

	ShowImguiDemoWindow();

	wnd.Gfx().EndFrame();
	fc.Reset();
}

void App::SpawnBackgroundControlWindow() noexcept
{
	if (ImGui::Begin("Background Colour")) {
		ImGui::Text("Background Colour");
		ImGui::SameLine();
		ImGui::ColorEdit3("", &bgColour.x);
	}
	ImGui::End();
}

void App::ShowImguiDemoWindow()
{
	if (showDemoWindow) {
		ImGui::ShowDemoWindow(&showDemoWindow);
	}
}

void App::PollInput(float dt)
{
	while (const auto e = wnd.kbd.ReadKey())
	{
		//Toggles Camera On and Off when Q is pressed
		if (e->IsPress() && e->GetCode() == KEY_Q) {
			if (wnd.CursorEnabled()) {
				wnd.DisableCursor();
				wnd.mouse.EnableRaw();
			}
			else
			{
				wnd.EnableCursor();
				wnd.mouse.DisableRaw();
			}
		}

		//Toggles Imgui On and Off when SPACE is pressed
		if (e->IsPress() && e->GetCode() == KEY_SYS_SPACE) {
			if (wnd.Gfx().IsImGuiEnabled()) {
				wnd.Gfx().DisableImgui();
			}
			else {
				wnd.Gfx().EnableImgui();
			}
		}
	}

	if (!wnd.CursorEnabled()) {
		if (wnd.kbd.KeyIsPressed(KEY_W)) {
			cam.Translate({ 0.0f,0.0f,dt });
		}
		if (wnd.kbd.KeyIsPressed(KEY_A)) {
			cam.Translate({ -dt,0.0f,0.0f });
		}
		if (wnd.kbd.KeyIsPressed(KEY_S)) {
			cam.Translate({ 0.0f,0.0f,-dt });
		}
		if (wnd.kbd.KeyIsPressed(KEY_D)) {
			cam.Translate({ dt,0.0f,0.0f });
		}
		if (wnd.kbd.KeyIsPressed(KEY_R)) {
			cam.Translate({ 0.0f,dt,0.0f });
		}
		if (wnd.kbd.KeyIsPressed(KEY_F)) {
			cam.Translate({ 0.0f,-dt,0.0f });
		}
		if (wnd.mouse.LeftIsPressed()) {
			wnd.EnableCursor();
		}
	}
}

