#include "Window.h"
#include <sstream>
#include "resource.h"

#include "Constants.h"
#include "WindowsThrowMacros.h"
#include "imgui/imgui_impl_win32.h"

//Window Class initialization
Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept : hInst(GetModuleHandle(nullptr))
{
	//Windows Class description
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32,32,0));
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));;
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

Window::Window(int width, int height, const char* name) : width(width), height(height)
{
	//Sets the size of the client area
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;

	//Sets the style of the window
	DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_MAXIMIZEBOX;

	//Adjusts the client area to the desired size
	AdjustWindowRect(&wr, style, FALSE);

	if (AdjustWindowRect(&wr, style, FALSE) == 0) {
		throw RSWND_LAST_EXCEPT();
	}

	//Create window and get hWnd
	hWnd = CreateWindow(
		WindowClass::GetName(), name,
		style,
		CW_USEDEFAULT, CW_USEDEFAULT, //Starting position
		wr.right - wr.left, //Width
		wr.bottom - wr.top, //Height
		nullptr, nullptr,
		WindowClass::GetInstance(),
		this //This is a long pointer to the Window instance 
		);

	if (hWnd == nullptr) {
		throw RSWND_LAST_EXCEPT();
	}

	ShowWindow(hWnd, SW_SHOWDEFAULT);

	//Init Imgui
	ImGui_ImplWin32_Init(hWnd);


	pGfx = std::make_unique<Graphics>(hWnd, width, height);
}

Window::~Window()
{
	ImGui_ImplWin32_Shutdown();
	DestroyWindow(hWnd); //Destroys the window
}

//only exists to setup the pointer to our instnace on the windows api side
LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (msg == WM_NCCREATE) { //if this is the first time the window is being created
		//extract a pointer to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam); 
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		//Set WinAPI data to store a pointer to the window class
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd)); //Lets you set data stored at the WindowsAPI side to desired data //stores a pointer to this class in the windows API
		//Set message procedure to normal handler now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk)); //since this instance has been set in WinAPI we can now change the window proc to HandleMessageThunk
		//forward message to the window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam); //Handle the new event
	}
	//message will now be handled by the new WindowProc which is WindowThunk
	return DefWindowProc(hWnd, msg, wParam, lParam); 
}

//adapts from WIN32 Call function to c++ member function
LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	//retrieve a pointer to a window class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); //Get the window pointer
	//forward message to window class handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam); //Calls the member function HandleMSG
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}

	const auto& imio = ImGui::GetIO();

	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0; //Stops Destroy Window from being called twice as the function now exits and then the destructor gets called
		break;
	//Clears keyboard state when the window is not in focus
	case WM_KILLFOCUS:
		kbd.ClearState();
		break;
	
	//Keyboard messages
	case WM_KEYDOWN:
		//Syskey command need to be handled to track alt key
	case WM_SYSKEYDOWN:
		if (imio.WantCaptureKeyboard) {
			break;
		}
		if (!(lParam & 0x40000000) || kbd.AutorepeatEnabled()) { //Filters Autorepeat //0x40000000 bit 30 (AutoRepeat)
			kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (imio.WantCaptureKeyboard) {
			break;
		}
		kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
	case WM_CHAR: //Text input
		if (imio.WantCaptureKeyboard) {
			break;
		}
		kbd.OnChar(static_cast<unsigned char>(wParam));
	
	//Mouse Messages
	case WM_MOUSEMOVE:
	{
		if (imio.WantCaptureKeyboard) {
			break;
		}
		const POINTS pt = MAKEPOINTS(lParam);
		if (pt.x > 0 && pt.x < width && pt.y >= 0 && pt.y < height) {
			mouse.OnMouseMove(pt.x, pt.y);
			if (!mouse.IsInWindow()) {
				SetCapture(hWnd);
				mouse.OnMouseEnter();
			}
		}
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON)) {
				mouse.OnMouseMove(pt.x, pt.y);
			}
			else
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
		}
	}
		break;
	case WM_LBUTTONDOWN:
	{
		if (imio.WantCaptureKeyboard) {
			break;
		}
		const POINTS pt = MAKEPOINTS(lParam); //lParam holds mouse coordinates
		mouse.OnLeftPressed(pt.x, pt.y);

		//Brings window to foreground object when clicked on
		SetForegroundWindow(hWnd);
	}
		break;
	case WM_RBUTTONDOWN:
	{
		if (imio.WantCaptureKeyboard) {
			break;
		}
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightPressed(pt.x, pt.y);
	}
		break;
	case WM_LBUTTONUP:
	{
		if (imio.WantCaptureKeyboard) {
			break;
		}
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnLeftReleased(pt.x, pt.y);
	}
		break;
	case WM_RBUTTONUP:
	{
		if (imio.WantCaptureKeyboard) {
			break;
		}
		const POINTS pt = MAKEPOINTS(lParam);
		mouse.OnRightReleased(pt.x, pt.y);
	}
		break;
	case WM_MOUSEWHEEL:
	{
		if (imio.WantCaptureKeyboard) {
			break;
		}
		const POINTS pt = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam); //wParam holds mouse wheel
		mouse.OnWheelDelta(pt.x, pt.y, delta);
	}
		break;
	}
	
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Window::SetTitle(const std::string& title)
{
	if (SetWindowText(hWnd, title.c_str()) == 0) {
		throw RSWND_LAST_EXCEPT();
	}
}

std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
	{
		if (msg.message == WM_QUIT) {
			return (int)msg.wParam;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return{};
}

Graphics& Window::Gfx()
{
	if (!pGfx) {
		throw RSWND_NOGFX_EXCEPT();
	}
	return *pGfx;
}

#pragma region Windows Exception
std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;

	//FormatMessage takes a HRESULT and returns detail on the error
	//This function returns the length of the error code
	const DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
		);
	if (nMsgLen == 0) { //if the message length is 0 then the error code is not known
		return "Unknown error code";
	}
	std::string errorString = pMsgBuf; //errorString memory will live after this function call //copies the error string into this string
	LocalFree(pMsgBuf); //release memory from pMsgBuffer
	return errorString; 
}
#pragma endregion

#pragma region Windows HrException
Window::HrException::HrException(int line, const char* file, HRESULT hr) noexcept : Exception(line, file), hr(hr) {}

const char* Window::HrException::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept {
	return "RedSky Window Exception";
}

HRESULT Window::HrException::GetErrorCode() const noexcept {
	return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept {
	return Exception::TranslateErrorCode(hr);
}
#pragma endregion

#pragma region Windows NoGfxException
const char* Window::NoGfxException::GetType() const noexcept {
	return "RedSky Window Exception [No Graphics]";
}
#pragma endregion


