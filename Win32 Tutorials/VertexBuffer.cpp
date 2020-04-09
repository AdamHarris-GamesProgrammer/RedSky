#include "VertexBuffer.h"

void VertexBuffer::Bind(Graphics& gfx) noexcept
{
	const UINT offset = 0u;
	GetContext(gfx)->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
}
