#pragma once

#include "include/cef_app.h"

// Implement application-level callbacks for the browser process.
class Runtime
	: public CefApp
	, public CefBrowserProcessHandler {
public:
	Runtime();

	// CefApp methods:
	CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
	void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;
	// TODO: Use OnBeforeCommandLineProcessing for auto-registering plugins

	// CefBrowserProcessHandler methods:
	void OnContextInitialized() override;

private:
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(Runtime);
};
