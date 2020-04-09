#include "App.h"
#include <sstream>
#include <iomanip>
#include "Constants.h"

App::App() : wnd(WINDOW_WIDTH, WINDOW_HEIGHT, "RedSky Demo Window")
{
}

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
	const float c = sin(timer.Peek()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	
	wnd.Gfx().DrawTestTriangle(-timer.Peek(),0.0f,0.0f);
	
	wnd.Gfx().EndFrame();
}
