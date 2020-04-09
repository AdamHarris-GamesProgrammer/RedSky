#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>

#include "Constants.h"

namespace wrl = Microsoft::WRL;

namespace dx = DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	//Direct3D gets the width and height of the window
	sd.BufferDesc.Width = WINDOW_WIDTH;
	sd.BufferDesc.Height = WINDOW_HEIGHT;
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
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));

	D3D11_DEPTH_STENCIL_DESC  dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	
	wrl::ComPtr<ID3D11DepthStencilState> pDSState;
	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

	pContext->OMSetDepthStencilState(pDSState.Get(), 1u);


	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = WINDOW_WIDTH;
	descDepth.Height = WINDOW_HEIGHT;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	GFX_THROW_INFO(pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil)); //filling the depth stencil

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;

	GFX_THROW_INFO(pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDSV));


	//Bind Render target
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

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
	pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::DrawTestTriangle(float angle, float x, float z)
{
	HRESULT hr;

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;

	struct Vertex {
		struct {
			float x, y, z;
		} pos;

	};

	//Cube coordinates
	Vertex vertices[] = {
		{-1.0f,-1.0f,-1.0f	},
		{1.0f, -1.0f, -1.0f	},
		{-1.0f,1.0f,-1.0f	},
		{1.0f,1.0f,-1.0f	},
		{-1.0f,-1.0f,1.0f	},
		{1.0f,-1.0f,1.0f	},
		{-1.0f,1.0f,1.0f	},
		{1.0f,1.0f,1.0f		},
	};

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;



	GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &sd, &pVertexBuffer));

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);

	//create indexed buffer
	const unsigned short indices[] = {
		0,2,1,	2,3,1,
		1,3,5,	3,7,5,
		2,6,3,	3,6,7,
		4,5,7,	4,7,6,
		0,4,2,	2,4,6,
		0,1,4,	1,5,4
	};

	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = { 0 };
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.BindFlags = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);

	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	GFX_THROW_INFO(pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));


	//& operator is used for filling with a pointer to a pointer
	//if the data just wants to be used then use the .GetAddressOf() function

	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);


	//constant buffer
	struct ConstantBuffer {
		dx::XMMATRIX transform; //4*4 floating point matrix //don't access it directly //optimized for SSMD
	};

	//Rotation 
	const ConstantBuffer cb = {
		{
			dx::XMMatrixTranspose( //one time transpose on the CPU side as opposed to using the row_major conversion in the vertex shader
				dx::XMMatrixRotationZ(angle) *
				dx::XMMatrixRotationX(angle) *
				dx::XMMatrixTranslation(x,0.0f, z + 4.0f) *
				dx::XMMatrixPerspectiveLH(1.0f,3.0f / 4.0f,0.5f,10.0f)
			)
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstantBuffer;

	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	//bind to vertex shader
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());

	struct ConstantBuffer2 {
		struct
		{
			float r;
			float g;
			float b;
			float a;
		} face_colors[6];
	};

	const ConstantBuffer2 cb2 = {
		{
			{1.0f,0.0f,1.0f},
			{1.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f},
			{1.0f,1.0f,0.0f},
			{0.0f,1.0f,1.0f},
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstantBuffer2;

	D3D11_BUFFER_DESC cbd2;
	cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd2.Usage = D3D11_USAGE_DEFAULT;
	cbd2.CPUAccessFlags = 0u;
	cbd2.MiscFlags = 0u;
	cbd2.ByteWidth = sizeof(cb2);
	cbd2.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd2 = {};
	csd2.pSysMem = &cb2;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd2, &csd2, &pConstantBuffer2));

	//bind to pixel shader
	pContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());


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
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	GFX_THROW_INFO(pDevice->CreateInputLayout(
		ied, std::size(ied), 
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&pInputLayout
	));

	pContext->IASetInputLayout(pInputLayout.Get());


	//Set Topology Technology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//configure viewport
	D3D11_VIEWPORT vp;
	vp.Width = WINDOW_WIDTH;
	vp.Height = WINDOW_HEIGHT;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);

	//Draw Vertex Buffers
	GFX_THROW_INFO_ONLY(pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u));
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
