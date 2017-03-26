#pragma once

#include <stddef.h>

template <typename T, size_t length>
inline size_t size(T(&arr)[length]) { return length; }

inline int C_strncmp(const char* left, const char* right, int n) { return strncmp(left, right, n); }
inline int C_strncmp(const wchar_t* left, const wchar_t* right, int n) { return wcsncmp(left, right, n); }

template <typename Char, size_t length>
inline Char* GetArg(Char* arg, const Char(&kSwitch)[length], bool loose = true) {
	if (C_strncmp(kSwitch, arg, size(kSwitch) - 1))
		return nullptr;

	auto cand = arg + size(kSwitch) - 1;
	if (loose) {
		if (*cand != 0 && *cand != '=')
			return nullptr;
	} else {
		if (*cand != '=')
			return nullptr;
	}
	return cand + 1;
}

template <typename Char, size_t length>
inline bool PickArg(const Char*& dst, Char* arg, const Char(&kSwitch)[length], bool loose = true) {
	if (dst) return false;
	dst = GetArg(arg, kSwitch, true);
	return !!dst;
}

#if defined(_WIN32)
#define JOIN2(A, B) A ## B
#define JOIN(A, B) JOIN2(A, B)
#define _L(X) JOIN(L, X)

template <typename T>
struct local
{
	struct free {
		void operator()(T ptr) {
			LocalFree(ptr);
		}
	};

	using ptr = std::unique_ptr<T, free>;
};

inline std::wstring GetAppDir() {
	wchar_t buffer[2048];
	GetModuleFileNameW(nullptr, buffer, size(buffer));
	auto slash = wcsrchr(buffer, LAUNCHER_DIRSEPC);
	if (!slash)
		slash = buffer;
	*slash = 0;
	return buffer;
}

inline std::wstring TruePath(const std::wstring& path) {
	wchar_t buffer[2048];
	GetFullPathName(path.c_str(), size(buffer), buffer, nullptr);
	return buffer;
}
#else
#define TruePath(X) X // TODO?
#define _L(X) X
#endif
