#include "Window.h"

#include <sstream>

//Entry point for windows
int CALLBACK WinMain( //CALLBACK is a modifier which edits how the parameters are passed to Windows, this passes the parameters on the stack
	_In_ HINSTANCE hInstance,			//Ignore this there is a function that gets this
	_In_opt_ HINSTANCE hPrevInstance,	//Always null since windows 3.0
	_In_ LPSTR lpCmdLine,				//single string for the command line
	_In_ int nShowCmd					//how the window should be shown for you on startup
	){

	try {
		Window wnd(800, 300, "RedSky Window 1 Test");

		//message pump
		MSG msg;
		BOOL gResult;
		while (gResult = (GetMessage(&msg, nullptr, 0, 0)) > 0) {
			TranslateMessage(&msg); //WM_CHAR is the main purpose of this function
			DispatchMessage(&msg); //passes to window procedure that is related to this message

			if (wnd.kbd.KeyIsPressed(VK_SPACE)) {
				MessageBox(nullptr, "Jump!", "Space Key Was Pressed", MB_OK | MB_ICONEXCLAMATION);
			}
			if (wnd.kbd.KeyIsPressed(VK_MENU)) {
				MessageBox(nullptr, "Menu", "Alt Key was Pressed", MB_OK | MB_ICONEXCLAMATION);
			}

			while (!wnd.mouse.IsEmpty()) {
				const auto e = wnd.mouse.Read();
				switch (e.GetType()) {
				case Mouse::Event::Type::Leave:
					wnd.SetTitle("Gone!");
					break;
				case Mouse::Event::Type::Move:
					std::ostringstream oss;
					oss << "Mouse moved to (" << e.GetX() << "," << e.GetY() << ")";
					wnd.SetTitle(oss.str());
					break;
				}
				
			}
		}

		if (gResult == -1) {
			return -1;
		}

		return msg.wParam; //error ID
	} 
	catch(const RedSkyException& e) {
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e) {
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...) {
		MessageBox(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;

}