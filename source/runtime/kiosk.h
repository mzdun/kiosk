#pragma once

#include <utility>
#include <string>
#include <unordered_map>

#include "include/cef_scheme.h"
#include "include/internal/cef_ptr.h"

void RegisterKioskScheme(CefRawPtr<CefSchemeRegistrar> registrar);
bool RegisterKioskApps(std::string const& root);
