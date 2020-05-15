#include "VertexBuffer.h"
#include "BindableCodex.h"

namespace Bind {

	VertexBuffer::VertexBuffer(Graphics& gfx, const rsexp::VertexBuffer& vbuf)
		:
		VertexBuffer(gfx, "?", vbuf)
	{}
	VertexBuffer::VertexBuffer(Graphics& gfx, const std::string& tag, const rsexp::VertexBuffer& vbuf)
		:
		stride((UINT)vbuf.GetLayout().Size()),
		tag(tag),
		layout(vbuf.GetLayout())
	{
		INFOMAN(gfx);

		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(vbuf.SizeBytes());
		bd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vbuf.GetData();
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer));
	}

	void VertexBuffer::Bind(Graphics& gfx) noexcept
	{
		const UINT offset = 0u;
		GetContext(gfx)->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
	}

	const rsexp::VertexLayout& VertexBuffer::GetLayout() const noexcept
	{
		return layout;
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::Resolve(Graphics& gfx, const std::string& tag,
		const rsexp::VertexBuffer& vbuf)
	{
		assert(tag != "?");
		return Codex::Resolve<VertexBuffer>(gfx, tag, vbuf);
	}
	std::string VertexBuffer::GenerateUID_(const std::string& tag)
	{
		using namespace std::string_literals;
		return typeid(VertexBuffer).name() + "#"s + tag;
	}
	std::string VertexBuffer::GetUID() const noexcept
	{
		return GenerateUID(tag);
	}

}

