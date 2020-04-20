#pragma once
#include "Bindable.h"

namespace Bind {
	class VertexShader : public Bindable
	{
	public:
		VertexShader(Graphics& gfx, const std::wstring& path);
		void Bind(Graphics& gfx) noexcept override;
		ID3DBlob* GetByteCode() const noexcept;

	protected:
		Microsoft::WRL::ComPtr<ID3DBlob> pByteCodeBlob;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	};
}


