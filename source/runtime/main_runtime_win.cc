// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <windows.h>
#include <shellapi.h>
#include <memory>

#include "runtime.h"
#include "ptr.h"
#include "include/cef_sandbox_win.h"

#include "main_runtime.h"
#include "../common/switches.h"

static constexpr wchar_t kRootSwitch[] = L"--" _L(ROOT_SWITCH);
static constexpr wchar_t kResourcesSwitch[] = L"--" _L(RESOURCES_SWITCH);
static constexpr wchar_t kLocalesSwitch[] = L"--" _L(LOCALES_SWITCH);

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
// #define CEF_USE_SANDBOX 1
#undef CEF_USE_SANDBOX

#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library is currently built with VS2013. It may not
// link successfully with other VS versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

// Entry point function for all processes.
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Enable High-DPI support on Windows 7 or newer.
  CefEnableHighDPISupport();

  void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  CefMainArgs main_args(hInstance);

  int exit_code = CefExecuteProcess(main_args, NULL, sandbox_info);
  if (exit_code >= 0)
    return exit_code;

  // Specify CEF global settings here.
  CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  {
	  int argc = 0;
	  local<LPWSTR[]>::ptr argv{ CommandLineToArgvW(GetCommandLineW(), &argc) };

	  UpdateSettings(settings, argc, argv.get(), kResourcesSwitch, kLocalesSwitch, kRootSwitch);
  }

  auto app = CefPtr<Runtime>();
  CefInitialize(main_args, settings, app.get(), sandbox_info);
  CefRunMessageLoop();
  CefShutdown();

  return 0;
}
