#pragma once

#include "Bindable.h"
#include <memory>
#include <unordered_map>

namespace Bind {
	class Codex {
	public:
		template<class T, typename...Params>
		static std::shared_ptr<Bindable> Resolve(Graphics& gfx, Params&&...p) noxnd {
			return Get.Resolve_<T>(gfx, std::forward<Params>(p)...);
		}
	private:
		std::shared_ptr<Bindable> Resolve_(const std::string& key) const noxnd {
			auto i = binds.find(key);
			if (i == binds.end()) {
				return {};
			}
			else
			{
				return i->second;
			}
		}
		void Store_(std::shared_ptr<Bindable> bind) {
			binds[bind->GetUID()] = std::move(bind);
		}
		static Codex& Get() {
			static Codex codex;
			return codex;
		}
	private:
		std::unordered_map<std::string, std::shared_ptr<Bindable>> binds;
	};
}