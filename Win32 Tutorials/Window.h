#pragma once

#include "RedSkyWin.h"
#include "RedSkyException.h"

#include "Keyboard.h"
#include "Mouse.h"

#include "Graphics.h"
#include <memory>

#include <optional>

class Window //encapsulates the creation, destruction and handling of events
{
public:
	class Exception : public RedSkyException {
	public:
		Exception(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept;
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;

	private:
		HRESULT hr;
	};

private:
	class WindowClass { // a window requires a window class 
	public:
		static const char* GetName() noexcept; //the GetName() function does not throw an exception
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept; //registers on the winAPI
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator=(const WindowClass&) = delete; //Disables automatically generated members like the copy constructor
		static constexpr const char* wndClassName = "RedSky Engine Window"; //constexpr: a constant expression must be initialized immediately 
		static WindowClass wndClass; //only one windows class needs to exist
		HINSTANCE hInst;
	};

public:
	Window(int width, int height, const char* name) ;
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	void SetTitle(const std::string& title);
	static std::optional<int> ProcessMessages();
	Graphics& Gfx();
private:
	//WinAPI dosent accept member functions, therefore a static function is used
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept; //LRESULT is just a long pointer, CALLBACK is just a __stdcall. 
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept; //__stdcall is a calling convention that means the stack is cleaned automatically when the function is over
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	Keyboard kbd;
	Mouse mouse;

private:
	int width;
	int height;
	HWND hWnd;
	std::unique_ptr<Graphics> pGfx;
};


#define RSWND_EXCEPT(hr) Window::Exception(__LINE__,__FILE__,hr);
#define RSWND_LAST_EXCEPT() Window::Exception(__LINE__,__FILE__,GetLastError())
