#include "App.h"

#include <sstream>

//Entry point for windows
int CALLBACK WinMain( //CALLBACK is a modifier which edits how the parameters are passed to Windows, this passes the parameters on the stack
	_In_ HINSTANCE hInstance,			//Ignore this there is a function that gets this
	_In_opt_ HINSTANCE hPrevInstance,	//Always null since windows 3.0
	_In_ LPSTR lpCmdLine,				//single string for the command line
	_In_ int nShowCmd					//how the window should be shown for you on startup
	){

	try {
		//creates an instance of the apps class and calls it Go function starting the game loop
		App{}.Go();

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