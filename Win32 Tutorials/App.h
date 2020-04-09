#pragma once
#include "Window.h"
#include "RedSkyTimer.h"

class App
{
public:
	App();
	~App();
	int Go();
private:
	void DoFrame();
private:
	Window wnd;
	RedSkyTimer timer;
	std::vector<std::unique_ptr<class Box>> boxes;
};

