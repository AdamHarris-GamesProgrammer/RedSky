#pragma once
#include "Bindable.h"
#include "GraphicsThrowMacros.h"

class VertexBuffer : public Bindable
{
public:
	template<class V>
	VertexBuffer(Graphics& gfx, const std::vector<V>& vertices) : stride(sizeof(V)) {
		INFOMAN(gfx);

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0u;
		bufferDesc.ByteWidth = UINT(sizeof(V) * vertices.size());
		bufferDesc.StructureByteStride = sizeof(V);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices.data();

		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bufferDesc, &sd, &pVertexBuffer));
	}
	void Bind(Graphics& gfx) noexcept override;

protected:
	UINT stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
};

