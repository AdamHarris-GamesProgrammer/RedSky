#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>

namespace wrl = Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG //if in debug mode //use the info manager to add additional messages to the error window 
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr) //calls set so only new messages are outputted 
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw Graphics::InfoException(__LINE__,__FILE__,v);}}
#else //if in release mode
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall) //throws with no information if in release mode
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException(__LINE__,__FILE__,(hr))
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

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

	//Gain access to texture sub resource in swap chain
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource),&pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));
}

void Graphics::EndFrame()
{
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

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float colour[] = { red,green,blue,1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), colour); //have to use the .get() method to get the address of the ComPtr
}

void Graphics::DrawTestTriangle()
{
	namespace wrl = Microsoft::WRL;

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;

	struct Vertex {
		float x, y;
	};

	//Triangle coordinates
	const Vertex vertices[] = {
		{0.0,0.5},
		{0.5,-0.5},
		{-0.5,-0.5}
	};

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA sd = { 0 };
	sd.pSysMem = vertices;

	HRESULT hr;

	GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &sd, &pVertexBuffer));

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;

	//& operator is used for filling with a pointer to a pointer
	//if the data just wants to be used then use the .GetAddressOf() function

	pContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
	

	//Pixel Shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

	//Vertex Shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
	//data, size of the data,class linkage (nullptr), output shader
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);

	//Set Input Layout
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;

	//POSITION: Has to match with the name in the VertexShader
	//DXGI_FORMAT_R32G32_FLOAT: saying the element consists of two 32bit floating point numbers
	//0: offset from the start of the element to this piece of data
	const D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	GFX_THROW_INFO(pDevice->CreateInputLayout(ied, std::size(ied), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout));
	pContext->IASetInputLayout(pInputLayout.Get());

	//Bind Render target
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);

	//configure viewport
	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);

	//Set Topology Technology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Draw Vertex Buffers
	GFX_THROW_INFO_ONLY(pContext->Draw((UINT)std::size(vertices), 0u));
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

std::string Graphics::InfoException::GetErrorInfo() const noexcept {
	return info;
}
#pragma endregion InfoException
