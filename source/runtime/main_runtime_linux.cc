// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "runtime.h"
#include "ptr.h"
#include "main_runtime.h"
#include "../common/switches.h"

#include <X11/Xlib.h>

#include "include/base/cef_logging.h"

static constexpr char kRootSwitch[] = "--" ROOT_SWITCH;
static constexpr char kResourcesSwitch[] = "--" RESOURCES_SWITCH;
static constexpr char kLocalesSwitch[] = "--" LOCALES_SWITCH;

namespace {

int XErrorHandlerImpl(Display *display, XErrorEvent *event) {
  LOG(WARNING)
        << "X error received: "
        << "type " << event->type << ", "
        << "serial " << event->serial << ", "
        << "error_code " << static_cast<int>(event->error_code) << ", "
        << "request_code " << static_cast<int>(event->request_code) << ", "
        << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display *display) {
  return 0;
}

}  // namespace


// Entry point function for all processes.
int main(int argc, char* argv[]) {
  CefMainArgs main_args(argc, argv);
  int exit_code = CefExecuteProcess(main_args, NULL, NULL);
  if (exit_code >= 0) {
    return exit_code;
  }

  // Install xlib error handlers so that the application won't be terminated
  // on non-fatal errors.
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);

  CefSettings settings;

  UpdateSettings(settings, argc, argv, kResourcesSwitch, kLocalesSwitch, kRootSwitch);

  auto app = CefPtr<Runtime>();
  CefInitialize(main_args, settings, app.get(), NULL);
  CefRunMessageLoop();
  CefShutdown();

  return 0;
}
