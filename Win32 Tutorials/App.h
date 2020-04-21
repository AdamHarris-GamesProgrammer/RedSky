#pragma once
#include "Window.h"
#include "RedSkyTimer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"
#include "Mesh.h"

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

private:
	ImguiManager imgui;
	Window wnd;
	RedSkyTimer timer;
	std::vector<std::unique_ptr<class Drawable>> drawables;
	std::vector<class Box*> boxes;
	float simSpeed = 1.0f;
	
	Camera cam;

	PointLight light;

	DirectX::XMFLOAT4 bgColour = { 0.0f,0.0f,0.0f, 1.0f };

	Model nano{ wnd.Gfx(), "Models\\nanosuit.obj" };

};
