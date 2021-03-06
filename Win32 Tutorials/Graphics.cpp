#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>
#include "GraphicsThrowMacros.h"
#include "Constants.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "DepthStencil.h"

//shorthand for Microsoft::WRL
namespace wrl = Microsoft::WRL;

//adds the d3d11 and D3DCompiler library to the project settings
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

Graphics::Graphics(HWND hWnd, int width, int height)
	: width(width), height(height)
{
	HRESULT hr;

	SetupSwapchainAndDevice(hWnd, width, height);
	SetupRenderTarget();

	//Bind Render target
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

	SetupViewport(width, height);

	ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());
}

#pragma region DirectX Setup
void Graphics::SetupSwapchainAndDevice(HWND& hWnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	//Direct3D gets the width and height of the window
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //Sets the colour format
	sd.BufferDesc.RefreshRate.Numerator = 0; //Pick the current system refresh rate
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //no scaling should be required 
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //Not specifying the scan line order, let the monitor decide
	sd.SampleDesc.Count = 1; //no Anti Aliasing
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //Sets this as the render target output
	sd.BufferCount = 1; //one back buffer and one front buffer
	sd.OutputWindow = hWnd; //Sets the window
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //swap effect discard is the standard effect
	sd.Flags = 0; //no flags currently

	UINT swapCreateFlags = 0u;

#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr;

	//Create device and swap chain
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	));
}

void Graphics::SetupRenderTarget()
{
	HRESULT hr;
	//Gain access to texture sub resource in swap chain
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));
}


void Graphics::SetupViewport(int width, int height)
{
	//configure viewport
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);
}
#pragma endregion DirectX Setup Functions

Graphics::~Graphics() {
	ImGui_ImplDX11_Shutdown();
}

void Graphics::BeginFrame(DirectX::XMFLOAT4 colour) noexcept
{
	if (imguiEnabled) {
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	const float color[] = { colour.x, colour.y, colour.z, 0.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

void Graphics::EndFrame()
{
	//Renders GUI if imgui is enabled
	if (imguiEnabled) {
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	HRESULT hr; //GFX_THROW_FAILED requires a local hresult variable

#ifndef NDEBUG
	infoManager.Set();
#endif // !NDEBUG

	if (FAILED(hr = pSwap->Present(1u, 0u))) { //if the buffer swap fails
		if (hr == DXGI_ERROR_DEVICE_REMOVED) { //unstable gpu error
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason()); //gets the reason for this error, usually caused due to hardware errors or drivers
		}
		else
		{
			throw GFX_EXCEPT(hr); //General purpose error
		}
	}
}

void Graphics::BindSwapBuffer() noexcept
{
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);
}

void Graphics::BindSwapBuffer(const DepthStencil& ds) noexcept
{
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), ds.pDepthStencilView.Get());
}

void Graphics::DrawIndexed(UINT count) noxnd
{
	GFX_THROW_INFO_ONLY(pContext->DrawIndexed(count, 0u, 0u));
}

//Graphics Exception Classes
#pragma region HrException
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs /*= {}*/) noexcept : Exception(line, file), hr(hr)
{
	for (const auto& m : infoMsgs) {
		info += m;
		info.push_back('\n');
	}

	if (!info.empty()) {
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << "(" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty()) { //if anything is in the info vector then we add it to the oss stream
		oss << "\n[Error Info]\n" << GetErrorInto() << std::endl << std::endl;
	}

	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "RedSky Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription(hr, buf, sizeof(buf));
	return buf;
}

std::string Graphics::HrException::GetErrorInto() const noexcept
{
	return info;
}
#pragma endregion HrException

#pragma region DeviceRemovedException
const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "RedSky Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

#pragma endregion DeviceRemovedException

#pragma region InfoException
Graphics::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept : Exception(line, file) {
	for (const auto& m : infoMsgs) {
		info += m;
		info.push_back('\n');
	}

	if (!info.empty()) {
		info.pop_back();
	}
}

const char* Graphics::InfoException::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept {
	return "RedSky Graphics Info Exception";
}
#pragma endregion InfoException
