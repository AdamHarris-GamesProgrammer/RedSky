#include "VertexBuffer.h"
namespace Bind {


	VertexBuffer::VertexBuffer(Graphics& gfx, const rsexp::VertexBuffer& vbuf)
		: stride((UINT)vbuf.GetLayout().Size()) 
	{
		INFOMAN(gfx);

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0u;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = UINT(vbuf.SizeBytes());
		bufferDesc.StructureByteStride = stride;

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vbuf.GetData();

		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bufferDesc, &sd, &pVertexBuffer));

	}

	void VertexBuffer::Bind(Graphics& gfx) noexcept
	{
		const UINT offset = 0u;
		GetContext(gfx)->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
	}
}

