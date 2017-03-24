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

	struct Ratio {
		unsigned num;
		unsigned denom;

		static unsigned GCD(unsigned a, unsigned b) {
			if (!b)
				return a;

			return GCD(b, a % b);
		}

		Ratio gcd() const {
			auto factor = GCD(num, denom);
			return{ num / factor, denom / factor };
		}

		template <typename T>
		T operator()(T in) const noexcept {
			return in * (T)num / (T)denom;
		}
	};

	struct ScreenRatio {
		Ratio x, y;
	};

	static ScreenRatio SystemDpiScaling();
private:
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(Runtime);
};
