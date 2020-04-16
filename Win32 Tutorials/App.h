#pragma once
#include "Window.h"
#include "RedSkyTimer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "PointLight.h"

class App
{
public:
	App();
	~App();
	int Go();
private:
	void DoFrame();

	void SpawnBackgroundControlWindow();
	void SpawnSpeedControlWindow();
private:
	ImguiManager imgui;
	Window wnd;
	RedSkyTimer timer;
	std::vector<std::unique_ptr<class Drawable>> drawables;

	float simSpeed = 1.0f;
	
	Camera cam;

	PointLight light;

	DirectX::XMFLOAT3 bgColour = { 0.0f,0.0f,0.0f };
	static constexpr size_t nDrawables = 180;
};
