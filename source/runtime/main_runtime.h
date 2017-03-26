#pragma once

#include "paths_runtime.h"
#include "../common/main_common.h"

template <typename Char, size_t len1, size_t len2, size_t len3>
inline void UpdateSettings(CefSettings& settings, int argc, Char* argv[], const Char(&kResourcesSwitch)[len1], const Char(&kLocalesSwitch)[len2], const Char(&kRootSwitch)[len3]) {
	const Char* resources_switch = nullptr;
	const Char* locales_switch = nullptr;
	const Char* root_switch = nullptr;

	for (int i = 1; i < argc; ++i) {
		auto arg = argv[i];
		if (PickArg(resources_switch, arg, kResourcesSwitch, true))
			continue;
		if (PickArg(locales_switch, arg, kLocalesSwitch, true))
			continue;
		if (PickArg(root_switch, arg, kRootSwitch))
			continue;
	}

	std::basic_string<Char> root;
	if (root_switch) {
		root = root_switch;
		if (root.empty() || root[root.length() - 1] != LAUNCHER_DIRSEPC)
			root.push_back(LAUNCHER_DIRSEPC);
	}
	else {
#if defined(OS_WIN)
		root = GetAppDir();
		root += _L(LAUNCHER_DIRSEP) _L(LAUNCHER_DIR_DEPTH);
#else
		root = LAUNCHER_PREFIX LAUNCHER_DIRSEP;
#endif
	}

	if (!resources_switch) {
		auto dir = TruePath(root + _L(LAUNCHER_DIR_RES));
		CefString(&settings.resources_dir_path) = dir;
		LOG(INFO) << "Setting resources to " << CefString(&settings.resources_dir_path).ToString();
	}

	if (!locales_switch) {
		auto dir = TruePath(root + _L(LAUNCHER_DIR_LOC));
		CefString(&settings.locales_dir_path) = dir;
		LOG(INFO) << "Setting locales to " << CefString(&settings.locales_dir_path).ToString();
	}
}
