#include "VertexShader.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include <typeinfo>

namespace Bind {
	using namespace std::string_literals;

	VertexShader::VertexShader(Graphics& gfx, const std::string& path) : path(path)
	{
		INFOMAN(gfx);
		GFX_THROW_INFO(D3DReadFileToBlob(std::wstring{ path.begin(), path.end() }.c_str(), &pByteCodeBlob));
		GFX_THROW_INFO(GetDevice(gfx)->CreateVertexShader(pByteCodeBlob->GetBufferPointer(), pByteCodeBlob->GetBufferSize(), nullptr, &pVertexShader));

	}

	void VertexShader::Bind(Graphics& gfx) noexcept
	{
		GetContext(gfx)->VSSetShader(pVertexShader.Get(), nullptr, 0u);
	}

	ID3DBlob* VertexShader::GetByteCode() const noexcept
	{
		return pByteCodeBlob.Get();
	}

	std::shared_ptr<Bind::Bindable> VertexShader::Resolve(Graphics& gfx, const std::string& path)
	{
		auto bind = Codex::Resolve(GenerateUID(path));
		if (!bind) {
			bind = std::make_shared<VertexShader>(gfx, path);
			Codex::Store(bind);
		}
		return bind;
	}

	std::string VertexShader::GenerateUID(const std::string& path)
	{
		return typeid(VertexShader).name() + "#"s + path;
	}

	std::string VertexShader::GetUID() const noexcept
	{
		return GenerateUID(path);
	}

}

