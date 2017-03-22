// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "embed.h"

#include <string>
#include <windows.h>
#include "resource.h"

#include "include/cef_browser.h"

void Embed::PlatformSetIcon(CefRefPtr<CefBrowser> browser)
{
	auto hwnd = browser->GetHost()->GetWindowHandle();
	auto small = LoadImage(GetModuleHandle(nullptr),
		MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		0);
	auto large = LoadImage(GetModuleHandle(nullptr),
		MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON,
		GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON),
		0);
	auto prev = SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)small);
	if (prev)
		DeleteObject((HGDIOBJ)prev);
	prev = SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)large);
	if (prev)
		DeleteObject((HGDIOBJ)prev);
}

void Embed::PlatformTitleChange(CefRefPtr<CefBrowser> browser,
	const CefString& title)
{
	CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
	SetWindowText(hwnd, std::wstring(title).c_str());
}
