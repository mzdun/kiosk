#include "runtime.h"

#include <string>

#include "embed.h"
#include "kiosk.h"
#include "ptr.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_path_util.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "version.h"

Runtime::Runtime()
{
}

void Runtime::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
	RegisterKioskScheme(registrar);
}

void Runtime::OnContextInitialized()
{
	CEF_REQUIRE_UI_THREAD();

	CefRefPtr<CefCommandLine> command_line =
		CefCommandLine::GetGlobalCommandLine();

	// Embed implements browser-level callbacks.
	auto handler = CefPtr<Embed>();

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	std::string url{"kiosk://panel/"};

	{
		CefString kiosk_dir;
		if (CefGetPath(PK_DIR_EXE, kiosk_dir)) {
			auto dir = kiosk_dir.ToString();
			if (!RegisterKioskApps({ dir, {{ "panel", "html" }} })) {
				url = "data:text/html,<title>Internal Error</title><h1>Failed to register kiosk apps";
			}
		}
		else {
			url = "data:text/html,<title>Internal Error</title><h1>Failed to setup kiosk";
		}
	}

	// Information used when creating the native window.
	CefWindowInfo window_info;
	window_info.width  = 800;
	window_info.height = 480;

#if defined(OS_WIN)
	// On Windows we need to specify certain flags that will be passed to
	// CreateWindowEx().
	window_info.SetAsPopup(NULL, "cef-runtime");
#endif

	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, nullptr);
}
