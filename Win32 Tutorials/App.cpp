#include "App.h"
#include <sstream>
#include <iomanip>
#include "Constants.h"
#include <memory>
#include "RedSkyMath.h"
#include <algorithm>
#include <random>
#include "Surface.h"
#include "AssTest.h"
#include "GDIPlusManager.h"
#include "imgui/imgui.h"
#include "Vertex.h"

GDIPlusManager gdipm;

namespace DX = DirectX;

App::App() : wnd(WINDOW_WIDTH, WINDOW_HEIGHT, "RedSky Demo Window"), light(wnd.Gfx())
{
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

	nano.Draw(wnd.Gfx());

	light.Draw(wnd.Gfx());

	SpawnBackgroundControlWindow();
	cam.SpawnControlWindow();
	light.SpawnControlWindow();
	ShowImguiDemoWindow();
	nano.ShowWindow();
	

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

void App::ShowImguiDemoWindow()
{
	static bool showDemoWindow = false;
	if (showDemoWindow) {
		ImGui::ShowDemoWindow(&showDemoWindow);
	}
}

