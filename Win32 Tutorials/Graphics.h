#pragma once

#include "RedSkyWin.h"
#include "RedSkyException.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <memory>
#include "DxgiInfoManager.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <random>

class Graphics
{
	friend class Bindable;
public:
	//Exception classes 
	class Exception : public RedSkyException { //Base graphics exception
		using RedSkyException::RedSkyException;
	};
	class HrException : public Exception { //graphics exception which has a HRESULT
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInto() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	class InfoException : public Exception {
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept { return info; }

	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException { //this is a particular type of exception
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};


public:
	//Constructor related methods
	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();

	//DirectX Setup
	void SetupSwapchainAndDevice(HWND& hWnd);
	void SetupRenderTarget();
	void SetupDepthStencil();
	void SetupViewport();

	//Swap Chain related functions
	void EndFrame();
	void BeginFrame(DirectX::XMFLOAT4 colour) noexcept;


	//Indexed Objects
	void DrawIndexed(UINT count) noexcept(!IS_DEBUG);

	//Getter/Setter for the projection
	void SetProjection(DirectX::FXMMATRIX proj) noexcept { projection = proj; }
	DirectX::XMMATRIX GetProjection() const noexcept { return projection; }

	//Getter/Setter for camera
	void SetCamera(DirectX::FXMMATRIX cam) noexcept { camera = cam; }
	DirectX::XMMATRIX GetCamera() const noexcept { return camera; }

	//Getter/Setter for ImGui
	void EnableImgui() noexcept { imguiEnabled = true; }
	void DisableImgui() noexcept { imguiEnabled = false; }
	bool IsImGuiEnabled() const noexcept { return imguiEnabled; }

private:
	DirectX::XMMATRIX camera;
	DirectX::XMMATRIX projection;

	bool imguiEnabled = true;
#ifndef NDEBUG 
	//if not in debug mode
	DxgiInfoManager infoManager;
#endif
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
};

