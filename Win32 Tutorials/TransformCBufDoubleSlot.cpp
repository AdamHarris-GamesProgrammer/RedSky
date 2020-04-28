#include "TransformCBufDoubleSlot.h"

namespace Bind {
	Bind::TransformCBufDoubleSlot::TransformCBufDoubleSlot(Graphics& gfx, const Drawable& parent, UINT slotV /*= 0u*/, UINT slotP /*= 0u*/)
		: TransformCbuf(gfx, parent, slotV)
	{
		if (!pPcbuf) {
			pPcbuf = std::make_unique<PixelConstantBuffer<Transforms>>(gfx, slotP);
		}
	}

	void Bind::TransformCBufDoubleSlot::Bind(Graphics& gfx) noexcept
	{
		const auto tf = GetTransforms(gfx);
		TransformCbuf::UpdateBindImpl(gfx, tf);
		UpdateBindImpl(gfx, tf);
	}

	void Bind::TransformCBufDoubleSlot::UpdateBindImpl(Graphics& gfx, const Transforms& tf) noexcept
	{
		pPcbuf->Update(gfx, tf);
		pPcbuf->Bind(gfx);
	}

	std::unique_ptr<Bind::PixelConstantBuffer<TransformCbuf::Transforms>> Bind::TransformCBufDoubleSlot::pPcbuf;
}