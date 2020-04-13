#include "App.h"
#include <sstream>
#include <iomanip>
#include "Constants.h"
#include "Box.h"
#include <memory>
#include "Melon.h"
#include "Pyramid.h"
#include "RedSkyMath.h"
#include <algorithm>
#include <random>
#include "Surface.h"
#include "GDIPlusManager.h"
#include "Sheet.h"
#include "SkinnedBox.h"
#include "imgui/imgui.h"

GDIPlusManager gdipm;


App::App() : wnd(WINDOW_WIDTH, WINDOW_HEIGHT, "RedSky Demo Window")
{
	class Factory {
	public:
		Factory(Graphics& gfx) : gfx(gfx) {}

		std::unique_ptr<Drawable> operator()() {
			switch (typedist(rng)) {
			case 0:
				return std::make_unique<Pyramid>(gfx, rng, adist, ddist, odist, rdist);
				break;

			case 1:
				return std::make_unique<Box>(gfx, rng, adist, ddist, odist, rdist, bdist);
				break;

			case 2:
				return std::make_unique<Melon>(gfx, rng, adist, ddist, odist, rdist, longdist, latdist);
				break;
			case 3:
				return std::make_unique<Sheet>(gfx, rng, adist, ddist, odist, rdist);
				break;
			case 4:
				return std::make_unique<SkinnedBox>(gfx, rng, adist, ddist, odist, rdist);
				break;
			default:
				assert(false && "[Error]: Invalid type in factory");
				return{};
				break;
			}
		}

	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_int_distribution<int> latdist{ 5,20 };
		std::uniform_int_distribution<int> longdist{ 10,40 };
		std::uniform_int_distribution<int> typedist{ 0,4 };
	};

	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, Factory{ wnd.Gfx() });

	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
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
	auto dt = timer.Mark();

	if (wnd.kbd.KeyIsPressed(VK_SPACE)) {
		wnd.Gfx().DisableImgui();
	}
	else
	{
		wnd.Gfx().EnableImgui();
	}
	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);


	for (auto& b : drawables) {
		b->Update(wnd.kbd.KeyIsPressed(VK_SPACE) ? 0.0f : dt);
		b->Draw(wnd.Gfx());
	}


	
	if (showDemo) {
		ImGui::ShowDemoWindow(&showDemo);
	}



	
	wnd.Gfx().EndFrame();
}
