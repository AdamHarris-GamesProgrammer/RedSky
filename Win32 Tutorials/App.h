#pragma once
#include "Window.h"
#include "RedSkyTimer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"
#include "Mesh.h"
#include "TestPlane.h"

class App
{
public:
	App();
	~App();
	int Go();
private:
	void DoFrame();

	void SpawnBackgroundControlWindow() noexcept;
	void ShowImguiDemoWindow();

	void PollInput(float dt);

private:
	ImguiManager imgui;
	Window wnd;
	RedSkyTimer timer;
	float simSpeed = 1.0f;
	
	bool showDemoWindow = false;

	Camera cam;

	PointLight light;

	DirectX::XMFLOAT4 bgColour = { 0.1f,0.1f,0.2f, 1.0f };

	Model goblin{ wnd.Gfx(), "Models\\gobber\\GoblinX.obj" };
	//Model wall{ wnd.Gfx(), "Models\\brick_wall\\brick_wall.obj" };
	//TestPlane tp{ wnd.Gfx(), 1.0f };

};
