#pragma once
#include "Window.h"
#include "RedSkyTimer.h"

class App
{
public:
	App();

	int Go();
private:
	void DoFrame();
private:
	Window wnd;
	RedSkyTimer timer;
};

