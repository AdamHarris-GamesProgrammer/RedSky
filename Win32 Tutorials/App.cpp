#include "App.h"
#include <sstream>
#include <iomanip>
#include "Constants.h"
#include "Box.h"
#include <memory>
#include "Pyramid.h"
#include "RedSkyMath.h"
#include <algorithm>
#include <random>
#include "Surface.h"
#include "AssTest.h"
#include "GDIPlusManager.h"
#include "SkinnedBox.h"
#include "imgui/imgui.h"
#include "Cylinder.h"
#include "Pyramid.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

GDIPlusManager gdipm;

namespace DX = DirectX;


App::App() : wnd(WINDOW_WIDTH, WINDOW_HEIGHT, "RedSky Demo Window"), light(wnd.Gfx())
{
	class Factory {
	public:
		Factory(Graphics& gfx) : gfx(gfx) {}

		std::unique_ptr<Drawable> operator()() {
			const DirectX::XMFLOAT3 mat = { cdist(rng), cdist(rng), cdist(rng) };
			const DirectX::XMFLOAT3 suzanneMat = { 0.25f, 0.75f, 0.5f };

			switch (sdist(rng)) {
			case 0:
				return std::make_unique<Box>(gfx, rng, adist, ddist, odist, rdist, bdist, mat);
			case 1:
				return std::make_unique<Cylinder>(gfx, rng, adist, ddist, odist, rdist, bdist, tdist);
			case 2:
				return std::make_unique<Pyramid>(gfx, rng, adist, ddist, odist, rdist, tdist);
			case  3:
				return std::make_unique<SkinnedBox>(gfx, rng, adist, ddist, odist, rdist);
			case  4:
				return std::make_unique<AssTest>(gfx, rng, adist, ddist, odist, rdist, suzanneMat, 1.5f);
			default:
				assert(false && "Invalid drawable option in factory class");
				break;
			}



		}

	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> sdist{ 0,4 };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_real_distribution<float> cdist{ 0.0f,1.0f };
		std::uniform_int_distribution<int> tdist{ 10,30 };
	};

	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, Factory{ wnd.Gfx() });

	//init boxes
	for (auto& pd : drawables) {
		if (auto pb = dynamic_cast<Box*>(pd.get())) {
			boxes.push_back(pb);
		}
	}

	wnd.Gfx().SetProjection(DX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
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

	if (wnd.kbd.KeyIsPressed(VK_SPACE)) {
		wnd.Gfx().DisableImgui();
	}
	else
	{
		wnd.Gfx().EnableImgui();
	}
	wnd.Gfx().BeginFrame(bgColour);
	wnd.Gfx().SetCamera(cam.GetMatrix());
	light.Bind(wnd.Gfx(), cam.GetMatrix());


	for (auto& b : drawables) {
		b->Update(wnd.kbd.KeyIsPressed(VK_SPACE) ? 0.0f : dt);
		b->Draw(wnd.Gfx());
	}
	light.Draw(wnd.Gfx());

	SpawnBackgroundControlWindow();
	SpawnSpeedControlWindow();
	cam.SpawnControlWindow();
	light.SpawnControlWindow();
	SpawnBoxWindowManagerWindow();
	SpawnBoxWindows();



	wnd.Gfx().EndFrame();
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

void App::SpawnSpeedControlWindow() noexcept
{
	if (ImGui::Begin("Simulation Speed")) {
		ImGui::Text("Simulation Speed");
		ImGui::SameLine();
		ImGui::SliderFloat("", &simSpeed, 0.0f, 6.0f, "%.4f", 3.2f);

	}
	ImGui::End();
}

void App::SpawnBoxWindowManagerWindow() noexcept
{
	if (ImGui::Begin("Boxes")) {
		using namespace std::string_literals;
		const auto preview = comboBoxIndex ? std::to_string(*comboBoxIndex) : "Choose a box"s;
		if (ImGui::BeginCombo("Box Number", preview.c_str())) {
			for (int i = 0; i < boxes.size(); i++) {
				const bool selected = *comboBoxIndex == i;
				if (ImGui::Selectable(std::to_string(i).c_str(), selected)) {
					comboBoxIndex = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Spawn Control Window") && comboBoxIndex) {
			boxControlIds.insert(*comboBoxIndex);
			comboBoxIndex.reset();
		}
	}
	ImGui::End();
}

void App::SpawnBoxWindows() noexcept
{
	for (auto i = boxControlIds.begin(); i != boxControlIds.end(); ) {
		if (!boxes[*i]->SpawnControlWindow(*i, wnd.Gfx())) {
			i = boxControlIds.erase(i);
		}
		else
		{
			i++;
		}
	}
}
