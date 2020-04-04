#include "Graphics.h"

#pragma comment(lib, "d3d11.lib")

Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	//Direct3D gets the width and height of the window
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
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

	//Create device and swap chain
	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	);

	//Gain access to texture sub resource in swap chain
	ID3D11Resource* pBackBuffer = nullptr;
	pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&pBackBuffer));
	pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pTarget);

	pBackBuffer->Release();
}

Graphics::~Graphics()
{
	if (pTarget != nullptr) {
		pTarget->Release();
	}
	if (pDevice != nullptr) {
		pDevice->Release();
	}
	if (pSwap != nullptr) {
		pSwap->Release();
	}
	if (pContext != nullptr) {
		pContext->Release();
	}
}

void Graphics::EndFrame()
{
	pSwap->Present(1u, 0u);
}
