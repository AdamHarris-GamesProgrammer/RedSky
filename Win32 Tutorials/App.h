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

	bool showDemo = true;
	static constexpr size_t nDrawables = 180;
};
