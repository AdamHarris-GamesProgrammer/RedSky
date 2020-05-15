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

namespace DX = DirectX;

App::App(const std::string& commandLine) :
	commandLine(commandLine),
	wnd(1280, 720, "RedSky Demo Window"), 
	scriptCommander(TokenizeQuoted(commandLine)),
	light(wnd.Gfx())
{
	TestDynamicConstant();

	//wall.SetRootTransform(DX::XMMatrixTranslation(-12.0f, 0.0f, 0.0f));
	//tp.SetPos({ 12.0f,0.0f,0.0f });
	//goblin.SetRootTransform(DX::XMMatrixTranslation(0.0f, 0.0f, -4.0f));
	//nano.SetRootTransform(DX::XMMatrixTranslation(0.0f, -7.0f, 6.0f));

	//bluePlane.SetPos(cam.GetPos());
	//redPlane.SetPos({-47.0f, 10.0f, 5.0f});

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


	//wall.Draw(wnd.Gfx());
	//tp.Draw(wnd.Gfx());
	//nano.Draw(wnd.Gfx());
	//goblin.Draw(wnd.Gfx());

	light.Submit(fc);
	cube.Submit(fc);
	cube2.Submit(fc);

	fc.Execute(wnd.Gfx());

	//bluePlane.Draw(wnd.Gfx());
	//redPlane.Draw(wnd.Gfx());

	SpawnBackgroundControlWindow();
	cam.SpawnControlWindow();
	light.SpawnControlWindow();
	//goblin.ShowWindow(wnd.Gfx(), "Goblin");
	//wall.ShowWindow(wnd.Gfx(), "Wall");
	//tp.SpawnControlWindow(wnd.Gfx());
	//nano.ShowWindow(wnd.Gfx(), "Nano");
	//sponza.ShowWindow(wnd.Gfx(), "Sponza");

	cube.SpawnControlWindow(wnd.Gfx(), "Cube 1");
	cube2.SpawnControlWindow(wnd.Gfx(), "Cube 2");

	//bluePlane.SpawnControlWindow(wnd.Gfx(), "Blue Plane");
	//redPlane.SpawnControlWindow(wnd.Gfx(), "Red Plane");
	
	//wall.ShowWindow("Wall");
	//tp.SpawnControlWindow(wnd.Gfx());
	
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
		//Toggles Camera On and Off when F is pressed
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

