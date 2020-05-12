#include "LayoutCodex.h"

namespace Dcb {

	Dcb::CookedLayout LayoutCodex::Resolve(Dcb::RawLayout&& layout) noxnd
	{
		auto sig = layout.GetSignature();
		auto& map = Get_().map;
		const auto i = map.find(sig);

		if (i != map.end()) {
			layout.ClearRoot();
			return { i->second };
		}
		auto result = map.insert({ std::move(sig), layout.DeliverRoot() });

		return { result.first->second };
	}

	Dcb::LayoutCodex& LayoutCodex::Get_() noexcept
	{
		static LayoutCodex codex;
		return codex;
	}

}