#pragma once

#include "RedSkyWin.h"

class Window
{
private:
	class WindowClass {
	public:
		static const char* GetName() noexcept; //the GetName() function does not throw an exception
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete; //Disables automatically generated members like the copy constructor
		static constexpr const char* wndClassName = "RedSky Engine Window"; //constexpr: a constant expression must be initialized immediately 
		static WindowClass wndClass;
		HINSTANCE hInst;
	};

public:
	Window(int width, int height, const char* name) noexcept;
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept; //LRESULT is just a long pointer, CALLBACK is just a __stdcall. 
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept; //__stdcall is a calling convention that means the stack is cleaned automatically when the function is over
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
	int width;
	int height;
	HWND hWnd;
};

