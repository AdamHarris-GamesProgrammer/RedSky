#include "GraphicsResource.h"

ID3D11DeviceContext* GraphicsResource::GetContext(Graphics& gfx) noexcept
{
	return gfx.pContext.Get();
}

ID3D11Device* GraphicsResource::GetDevice(Graphics& gfx) noexcept
{
	return gfx.pDevice.Get();
}

DxgiInfoManager& GraphicsResource::GetInfoManager(Graphics& gfx)
{
#ifndef NDEBUG
	return gfx.infoManager;
#else
	throw std::logic_error("[ERROR]: Tried accessing gfx.infoManager in release mode");
#endif // !NDEBUG

}
