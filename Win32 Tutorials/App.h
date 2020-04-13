#pragma once
#include "Window.h"
#include "RedSkyTimer.h"
#include "ImguiManager.h"

class App
{
public:
	App();
	~App();
	int Go();
private:
	void DoFrame();
private:
	ImguiManager imgui;
	Window wnd;
	RedSkyTimer timer;
	std::vector<std::unique_ptr<class Drawable>> drawables;

	float simSpeed = 1.0f;

	float r = 0.07f;
	float g = 0.0f;
	float b = 0.12f;

	float colour[3] = { 0.07f,0.0f,0.12f };
	static constexpr size_t nDrawables = 180;
};
