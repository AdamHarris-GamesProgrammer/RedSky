#pragma once
#include "Bindable.h"
#include "Vertex.h"

namespace Bind {
	class InputLayout : public Bindable
	{
	public:
		InputLayout(Graphics& gfx,
			rsexp::VertexLayout layout_in,
			ID3DBlob* pVertexShaderByteCode);
		void Bind(Graphics& gfx) noexcept override;
		static std::shared_ptr<InputLayout> Resolve(Graphics& gfx,
			const rsexp::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode);
		static std::string GenerateUID(const rsexp::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode = nullptr);
		std::string GetUID() const noexcept override;
	protected:
		rsexp::VertexLayout layout;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	};
}

