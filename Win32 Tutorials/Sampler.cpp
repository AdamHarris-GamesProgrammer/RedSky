#include "Sampler.h"
#include "GraphicsThrowMacros.h"

namespace Bind {
	Sampler::Sampler(Graphics& gfx)
	{
		INFOMAN(gfx);

		D3D11_SAMPLER_DESC sampleDesc = {};
		sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&sampleDesc, &pSampler));
	}

	void Sampler::Bind(Graphics& gfx) noexcept
	{
		GetContext(gfx)->PSSetSamplers(0, 1, pSampler.GetAddressOf());
	}
}