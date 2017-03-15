#pragma once

#include <utility>
#include <string>
#include <unordered_map>

#include "include/cef_scheme.h"
#include "include/internal/cef_ptr.h"

struct KioskConf {
	const std::string root;
	const std::unordered_map<std::string, std::string> apps;
};

void RegisterKioskScheme(CefRawPtr<CefSchemeRegistrar> registrar);
bool RegisterKioskApps(KioskConf conf);
