#include "InputLayout.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include "Vertex.h"

namespace Bind {
	InputLayout::InputLayout(Graphics& gfx, rsexp::VertexLayout layout_in, ID3DBlob* pVertexShaderByteCode)
		: layout(std::move(layout_in))
	{
		INFOMAN(gfx);

		const auto d3dLayout = layout.GetD3DLayout();

		GFX_THROW_INFO(GetDevice(gfx)->CreateInputLayout(
			d3dLayout.data(), (UINT)d3dLayout.size(),
			pVertexShaderByteCode->GetBufferPointer(),
			pVertexShaderByteCode->GetBufferSize(),
			&pInputLayout
		));
	}

	void InputLayout::Bind(Graphics& gfx) noexcept
	{
		GetContext(gfx)->IASetInputLayout(pInputLayout.Get());
	}

	std::shared_ptr<Bind::Bindable> InputLayout::Resolve(Graphics& gfx, const rsexp::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode)
	{
		return Codex::Resolve<InputLayout>(gfx, layout, pVertexShaderBytecode);
	}

	std::string InputLayout::GenerateUID(const rsexp::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode /*= nullptr*/)
	{
		using namespace std::string_literals;
		return typeid(InputLayout).name() + "#"s + layout.GetCode();
	}

	std::string InputLayout::GetUID() const noexcept
	{
		return GenerateUID(layout);
	}

}
