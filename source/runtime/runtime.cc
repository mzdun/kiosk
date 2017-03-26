#include "runtime.h"

#include <string>

#include "embed.h"
#include "kiosk.h"
#include "paths_runtime.h"
#include "ptr.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_path_util.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "template.h"
#include "version.h"

#include "../common/switches.h"

Runtime::Runtime()
{
}

void Runtime::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
	RegisterKioskScheme(registrar);
}

void Runtime::OnBeforeCommandLineProcessing(
	const CefString& process_type,
	CefRefPtr<CefCommandLine> command_line)
{
	if (!process_type.empty()) {
		LOG(INFO) << process_type.ToString();
		return;
	}

	// TODO: plugins
}

std::string get_kiosk_dir() {
	CefRefPtr<CefCommandLine> command_line =
		CefCommandLine::GetGlobalCommandLine();

	if (command_line->HasSwitch(KIOSK_SWITCH))
		return command_line->GetSwitchValue(KIOSK_SWITCH);

	if (command_line->HasSwitch(ROOT_SWITCH)) {
		return command_line->GetSwitchValue(ROOT_SWITCH).ToString()
			+ LAUNCHER_DIRSEP LAUNCHER_DIR_APPS;
	}

#if defined(OS_WIN)
	CefString app_dir;
	if (CefGetPath(PK_DIR_EXE, app_dir))
		return app_dir.ToString() + LAUNCHER_DIRSEP LAUNCHER_DIR_DEPTH LAUNCHER_DIR_APPS;

	return {};
#else
	return LAUNCHER_PREFIX LAUNCHER_DIRSEP LAUNCHER_DIR_APPS;
#endif
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

	std::string url{"kiosk://default/"};
	if (command_line->HasSwitch(APP_SWITCH)) {
		url = "kiosk://" +
			command_line->GetSwitchValue(APP_SWITCH).ToString() +
			"/";
	}

	{
		std::string kiosk_dir = get_kiosk_dir();
		if (!kiosk_dir.empty()) {
			if (!RegisterKioskApps(kiosk_dir)) {
				url = "data:text/html," + response_template("Internal Error", "<p>Failed to register kiosk apps</p>");
			}
		} else {
			url = "data:text/html," + response_template("Internal Error", "<p>Failed to setup kiosk apps</p>");
		}
	}

	// Information used when creating the native window.
	CefWindowInfo window_info;

#if defined(OS_WIN)
	// On Windows we need to specify certain flags that will be passed to
	// CreateWindowEx().
	window_info.SetAsPopup(NULL, "cef-runtime");
#endif

	auto scale = SystemDpiScaling();
	window_info.width = scale.x(800);
	window_info.height = scale.y(480);

#if defined(OS_WIN)
	{
		RECT r{ 0, 0, window_info.width, window_info.height };
		AdjustWindowRect(&r, window_info.style, FALSE);
		window_info.width = r.right - r.left;
		window_info.height = r.bottom - r.top;
	}
#endif
	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, nullptr);
}
