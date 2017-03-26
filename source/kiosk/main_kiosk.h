#pragma once

#include "paths_kiosk.h"
#include "../common/main_common.h"

template <typename Char, size_t len1, size_t len2, size_t len3>
std::basic_string<Char> FindRuntime(int argc, Char* argv[], const Char(&kCefSwitch)[len1], const Char(&kRuntimeSwitch)[len2], const Char(&kRootSwitch)[len3]) {
	const Char* cef_path = nullptr;
	const Char* runtime_path = nullptr;
	const Char* root_switch = nullptr;
	for (int i = 1; i < argc; ++i) {
		auto arg = argv[i];
		if (PickArg(cef_path, arg, kCefSwitch))
			continue;
		if (PickArg(runtime_path, arg, kRuntimeSwitch))
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
#if defined(_WIN32)
		root = GetAppDir();
		root += _L(LAUNCHER_DIRSEP) _L(LAUNCHER_DIR_DEPTH);
#else
		root = LAUNCHER_PREFIX LAUNCHER_DIRSEP;
#endif
	}

	bool TryLoad(const std::basic_string<Char>&);
	if (cef_path) {
		if (!TryLoad(cef_path))
			return {};
	} else {
		auto cef = TruePath(root
			+ _L(LAUNCHER_DIR_LIB) _L(LAUNCHER_DIRSEP) _L(LAUNCHER_LIBCEF)
		);
		if (!TryLoad(cef))
			return {};
	}

	return runtime_path ? runtime_path : root + _L(LAUNCHER_DIR_RT) _L(LAUNCHER_DIRSEP) _L(LAUNCHER_RUNTIME);
}
