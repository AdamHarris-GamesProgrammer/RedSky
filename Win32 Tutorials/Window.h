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
		using RedSkyException::RedSkyException;
	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
	};
	class HrException : public Exception {
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept; //Exception with a HRSULT
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;

		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	class NoGfxException : public Exception { //when graphics is trying to be retrieved but the pointer isnt valid
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
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
	static std::optional<int> ProcessMessages() noexcept;
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


#define RSWND_EXCEPT(hr) Window::HrException(__LINE__,__FILE__,(hr));
#define RSWND_LAST_EXCEPT() Window::HrException(__LINE__,__FILE__,GetLastError())
#define RSWND_NOGFX_EXCEPT() Window::NoGfxException(__LINE__,__FILE__)
