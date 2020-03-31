#include "Window.h"

//Entry point for windows
int CALLBACK WinMain( //CALLBACK is a modifier which edits how the parameters are passed to Windows, this passes the parameters on the stack
	_In_ HINSTANCE hInstance,			//Ignore this there is a function that gets this
	_In_opt_ HINSTANCE hPrevInstance,	//Always null since windows 3.0
	_In_ LPSTR lpCmdLine,				//single string for the command line
	_In_ int nShowCmd					//how the window should be shown for you on startup
	){

	try {
		Window wnd(800, 300, "RedSky Window 1 Test");
		Window wnd1(600, 600, "RedSky Window 2 Test");
		Window wnd2(400, 120, "RedSky Window 3 Test");
		Window wnd3(200, 600, "RedSky Window 4 Test");

		//message pump
		MSG msg;
		BOOL gResult;
		while (gResult = (GetMessage(&msg, nullptr, 0, 0)) > 0) {
			TranslateMessage(&msg); //WM_CHAR is the main purpose of this function
			DispatchMessage(&msg); //passes to window procedure that is related to this message
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