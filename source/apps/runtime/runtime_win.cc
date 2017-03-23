#include "runtime.h"

#include <windows.h>

class WindowDC
{
	HWND wnd_;
	HDC dc_;
public:
	HDC get() const { return dc_; }

	WindowDC(HWND wnd) : wnd_{ wnd }, dc_{ ::GetWindowDC(wnd) }
	{
	}

	~WindowDC()
	{
		::ReleaseDC(wnd_, dc_);
	}
};

Runtime::ScreenRatio Runtime::SystemDpiScaling()
{
	WindowDC dc{ ::GetDesktopWindow() };
	ScreenRatio out;
	out.x = Ratio{ (unsigned)::GetDeviceCaps(dc.get(), LOGPIXELSX), 96u }.gcd();
	out.y = Ratio{ (unsigned)::GetDeviceCaps(dc.get(), LOGPIXELSY), 96u }.gcd();
	return out;
}

