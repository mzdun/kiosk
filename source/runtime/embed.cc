// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "embed.h"

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "template.h"

namespace {

	Embed* g_instance = NULL;

}  // namespace

Embed::Embed()
	: is_closing_(false)
{
	DCHECK(!g_instance);
	g_instance = this;
}

Embed::~Embed() {
	g_instance = NULL;
}

// static
Embed* Embed::GetInstance() { return g_instance; }

void Embed::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	CEF_REQUIRE_UI_THREAD();

	// Set the title of the window using platform APIs.
	PlatformTitleChange(browser, title);
}

void Embed::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	// Add to the list of existing browsers.
	browser_list_.push_back(browser);
	PlatformSetIcon(browser);
}

bool Embed::DoClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed destription of this
	// process.
	if (browser_list_.size() == 1) {
		// Set a flag to indicate that the window close should be allowed.
		is_closing_ = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void Embed::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	// Remove from the list of existing browsers.
	using std::begin; using std::end;
	auto it = std::find_if(begin(browser_list_), end(browser_list_), [=](auto const& ref) {
		return ref->IsSame(browser);
	});
	if (it != end(browser_list_))
		browser_list_.erase(it);

	if (browser_list_.empty()) {
		// All browser windows have closed. Quit the application message loop.
		CefQuitMessageLoop();
	}
}

void Embed::OnLoadError(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl)
{
	CEF_REQUIRE_UI_THREAD();

	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;

	auto err = std::string(errorText) + " (" + std::to_string(errorCode) + ")";
	frame->LoadString(response_template(err, "Failed to load URL", "<p>Url: <tt>" + std::string(failedUrl) + "</tt>.</p><p>Error: " + err + ".</p>"), failedUrl);
}

void Embed::CloseAllBrowsers(bool force_close)
{
	if (!CefCurrentlyOn(TID_UI)) {
		CefPostTask(TID_UI,
			base::Bind(&Embed::CloseAllBrowsers, this, force_close));
		return;
	}

	if (browser_list_.empty())
		return;

	BrowserList::const_iterator it = browser_list_.begin();
	for (; it != browser_list_.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);
}
