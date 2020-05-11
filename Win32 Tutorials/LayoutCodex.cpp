#include "LayoutCodex.h"

namespace Dcb {

	Dcb::Layout LayoutCodex::Resolve(Dcb::Layout& layout) noxnd
	{
		layout.Finalize();
		auto sig = layout.GetSignature();
		auto& map = Get_().map;
		const auto i = map.find(sig);

		if (i != map.end()) {
			return { i->second };
		}
		auto result = map.insert({ std::move(sig), layout.ShareRoot() });

		return { result.first->second };
	}

	Dcb::LayoutCodex& LayoutCodex::Get_() noexcept
	{
		static LayoutCodex codex;
		return codex;
	}

}