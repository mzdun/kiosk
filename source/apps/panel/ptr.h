#pragma once

#include "include/cef_client.h"

template <typename T, typename ... Args>
inline CefRefPtr<T> CefPtr(Args&& ... args) {
	return new T(std::forward<Args>(args)...);
}
