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

	auto err = std::string(errorText);
	// Display a load error message.
	std::stringstream ss;
	ss << R"(<html><head><style>
body{font-family:sans-serif;color:#f0f0f0;background:black;margin:0;padding:0}
.content{display:grid;grid-template-rows:min-content 1fr min-content;grid-template-columns:4em 1fr;height:100%}
@media(min-width:800px){.content{width:800px;margin:0 auto}}
header{grid-row:1;grid-column:1/span 2;grid-template-columns:4em 1fr;display:grid}
header>svg{display:inline-block;grid-row:1;grid-column:1;justify-self:center;align-self:center}
h1{grid-row:1;grid-column:2;margin:.5em}
p{margin:1em}
content{grid-row:2;grid-column:2}
footer{grid-row:3;grid-column:2;font-size:50%;text-align:center}
hr { width:60%;border:solid .5px silver;text-align:left }</style><title>)" << err << " (" << errorCode << R"()</title></head>
<body><div class='content'>
<header><svg xmlns="http://www.w3.org/2000/svg" width="3em" height="3em" viewBox="0 0 32 32" version="1.1"	style='stroke-linejoin:round;stroke-linecap:round;stroke:#3faade;fill:none;stroke-width:3'><path stroke-width='6' d='M6,14.4L16,4.4L26,14.4L26,24.4L6,24.4z'/><path stroke='white' d='M6,14.4L16,4.4L26,14.4L26,24.4L6,24.4z'/></svg><h1>Failed to load URL <tt>)" << std::string(failedUrl) << R"(</tt>.</h1></header>
<content><p>Error: )" << err << " (" << errorCode << R"()</p></content>
<footer><hr/><p>Powered by Kiosk and Chromium</p></footer>
</div></body></html>)";
	frame->LoadString(ss.str(), failedUrl);
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
