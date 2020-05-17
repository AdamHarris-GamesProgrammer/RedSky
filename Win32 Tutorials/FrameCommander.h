#pragma once
#include <array>
#include "BindableCommon.h"
#include "Graphics.h"
#include "Job.h"
#include "Pass.h"
#include "PerformanceLog.h"

class FrameCommander {
public:
	void Accept(Job job, size_t target) noexcept {
		passes[target].Accept(job);
	}

	void Execute(Graphics& gfx) const noxnd {
		using namespace Bind;

		Stencil::Resolve(gfx, Stencil::Mode::Off)->Bind(gfx);
		passes[0].Execute(gfx);

		Stencil::Resolve(gfx, Stencil::Mode::Write)->Bind(gfx);
		NullPixelShader::Resolve(gfx)->Bind(gfx);
		passes[1].Execute(gfx);

		PerfLog::Start("Begin");
		Stencil::Resolve(gfx, Stencil::Mode::Mask)->Bind(gfx);
		struct SolidColorBuffer {
			DirectX::XMFLOAT4 color = { 0.0f,0.0f,0.0f, 1.0f };
		} scb;
		PixelConstantBuffer<SolidColorBuffer>::Resolve(gfx, scb, 1u)->Bind(gfx);
		PerfLog::Mark("Resolve 2x");
		passes[2].Execute(gfx);
	}

	void Reset() noexcept {
		for (auto& p : passes) {
			p.Reset();
		}
	}

private:
	std::array<Pass, 3> passes;
};