#include "DynamicConstant.h"
#include "LayoutCodex.h"
#include <cstring>

namespace DX = DirectX;

void TestDynamicConstant() {
	using namespace std::string_literals;
	// data roundtrip tests
	{
		Dcb::RawLayout s;
		s.Add<Dcb::Struct>("testStruct"s);
		s["testStruct"s].Add<Dcb::Float3>("testStructFloat3"s);
		s["testStruct"s].Add<Dcb::Float>("testStructFloat"s);
		s.Add<Dcb::Float>("testFloat"s);
		s.Add<Dcb::Array>("testArray"s);
		s["testArray"s].Set<Dcb::Struct>(4);
		s["testArray"s].T().Add<Dcb::Float3>("testArrayFloat3"s);
		s["testArray"s].T().Add<Dcb::Array>("testArrayFloat3Float"s);
		s["testArray"s].T()["testArrayFloat3Float"s].Set<Dcb::Float>(6);
		s["testArray"s].T().Add<Dcb::Array>("testArrayInArray"s);
		s["testArray"s].T()["testArrayInArray"s].Set<Dcb::Array>(6);
		s["testArray"s].T()["testArrayInArray"s].T().Set<Dcb::Matrix>(4);
		s["testArray"s].T().Add<Dcb::Bool>("testArrayBool"s);
		auto b = Dcb::Buffer::Make(std::move(s));

		const auto sig = b.GetLayout().GetSignature();

		{
			auto exp = 42.0f;
			b["testFloat"s] = exp;
			float act = b["testFloat"s];
			assert(act == exp);
		}
		{
			auto exp = 420.0f;
			b["testStruct"s]["testStructFloat"s] = exp;
			float act = b["testStruct"s]["testStructFloat"s];
			assert(act == exp);
		}
		{
			auto exp = 111.0f;
			b["testArray"s][2]["testArrayFloat3Float"s][5] = exp;
			float act = b["testArray"s][2]["testArrayFloat3Float"s][5];
			assert(act == exp);
		}
		{
			auto exp = DirectX::XMFLOAT3{ 69.0f,0.0f,0.0f };
			b["testStruct"s]["testStructFloat3"s] = exp;
			DX::XMFLOAT3 act = b["testStruct"s]["testStructFloat3"s];
			assert(!std::memcmp(&exp, &act, sizeof(DirectX::XMFLOAT3)));
		}
		{
			DirectX::XMFLOAT4X4 exp;
			DX::XMStoreFloat4x4(
				&exp,
				DX::XMMatrixIdentity()
			);
			b["testArray"s][2]["testArrayInArray"s][5][3] = exp;
			DX::XMFLOAT4X4 act = b["testArray"s][2]["testArrayInArray"s][5][3];
			assert(!std::memcmp(&exp, &act, sizeof(DirectX::XMFLOAT4X4)));
		}
		{
			auto exp = true;
			b["testArray"s][2]["testArrayBool"s] = exp;
			bool act = b["testArray"s][2]["testArrayBool"s];
			assert(act == exp);
		}
		{
			auto exp = false;
			b["testArray"s][2]["testArrayBool"s] = exp;
			bool act = b["testArray"s][2]["testArrayBool"s];
			assert(act == exp);
		}
	}
	// size test testArray of testArrays
	{
		Dcb::RawLayout s;
		s.Add<Dcb::Array>("testArray");
		s["testArray"s].Set<Dcb::Array>(6);
		s["testArray"s].T().Set<Dcb::Matrix>(4);
		auto b = Dcb::Buffer::Make(std::move(s));

		auto act = b.GetSizeInBytes();
		assert(act == 16u * 4u * 4u * 6u);
	}
	// size test testArrayay of structs with padding
	{
		Dcb::RawLayout s;
		s.Add<Dcb::Array>("testArray");
		s["testArray"s].Set<Dcb::Struct>(6);
		s["testArray"s].T().Add<Dcb::Float2>("a");
		s["testArray"s].T().Add<Dcb::Float3>("b"s);
		auto b = Dcb::Buffer::Make(std::move(s));

		auto act = b.GetSizeInBytes();
		assert(act == 16u * 2u * 6u);
	}
	// size test testArrayay of primitive that needs padding
	{
		Dcb::RawLayout s;
		s.Add<Dcb::Array>("testArray");
		s["testArray"s].Set<Dcb::Float3>(6);
		auto b = Dcb::Buffer::Make(std::move(s));

		auto act = b.GetSizeInBytes();
		assert(act == 16u * 6u);
	}
	// testing CookedLayout
	{
		Dcb::RawLayout s;
		s.Add<Dcb::Array>("arr");
		s["arr"].Set<Dcb::Float3>(6);
		auto cooked = Dcb::LayoutCodex::Resolve(std::move(s));
		// raw is cleared after donating
		s.Add<Dcb::Float>("arr");
		// fails to compile, cooked returns const&
		// cooked["arr"].Add<Dcb::Float>("buttman");
		auto b1 = Dcb::Buffer::Make(cooked);
		b1["arr"][0] = DX::XMFLOAT3{ 69.0f,0.0f,0.0f };
		auto b2 = Dcb::Buffer::Make(cooked);
		b2["arr"][0] = DX::XMFLOAT3{ 420.0f,0.0f,0.0f };
		assert(static_cast<DX::XMFLOAT3>(b1["arr"][0]).x == 69.0f);
		assert(static_cast<DX::XMFLOAT3>(b2["arr"][0]).x == 420.0f);
	}
}
