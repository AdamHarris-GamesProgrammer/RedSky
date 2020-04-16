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
#include "GDIPlusManager.h"
#include "SkinnedBox.h"
#include "imgui/imgui.h"
#include "Cylinder.h"

GDIPlusManager gdipm;

namespace DX = DirectX;


App::App() : wnd(WINDOW_WIDTH, WINDOW_HEIGHT, "RedSky Demo Window"), light(wnd.Gfx())
{
	class Factory {
	public:
		Factory(Graphics& gfx) : gfx(gfx) {}

		std::unique_ptr<Drawable> operator()() {
			const DirectX::XMFLOAT3 mat = { cdist(rng), cdist(rng), cdist(rng) };

			switch (sdist(rng)) {
			case 0:
				return std::make_unique<Box>(gfx, rng, adist, ddist, odist, rdist, bdist, mat);
				break;
			case 1:
				return std::make_unique<Cylinder>(gfx, rng, adist, ddist, odist, rdist, bdist, tdist);
				break;
			default:
				assert(false && "Invalid drawable option in factory class");
				break;
			}

			

		}

	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> sdist{ 0,1 };
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

	wnd.Gfx().EndFrame();
}

void App::SpawnBackgroundControlWindow()
{
	if (ImGui::Begin("Background Colour")) {
		ImGui::Text("Background Colour");
		ImGui::SameLine();
		ImGui::ColorEdit3("", &bgColour.x);
	}
	ImGui::End();
}

void App::SpawnSpeedControlWindow()
{
	if (ImGui::Begin("Simulation Speed")) {
		ImGui::Text("Simulation Speed");
		ImGui::SameLine();
		ImGui::SliderFloat("", &simSpeed, 0.0f, 6.0f, "%.4f", 3.2f);

	}
	ImGui::End();
}
