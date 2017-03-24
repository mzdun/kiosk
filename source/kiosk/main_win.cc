#include <windows.h>
#include <paths.h>
#include <tchar.h>
#include <memory>
#include <string>
#include <vector>

#define JOIN2(A, B) A ## B
#define JOIN(A, B) JOIN2(A, B)
#define LAUNCHER_DIR_LIBW JOIN(L, LAUNCHER_DIR_LIB)

#define DIRSEPW L"\\"
#define LAUNCHER_LIBCEFW   JOIN(L, LAUNCHER_LIBCEF)
#define LAUNCHER_RUNTIMEW  JOIN(L, LAUNCHER_RUNTIME)
#define LAUNCHER_DIR_RTW  JOIN(L, LAUNCHER_DIR_RT)

static constexpr wchar_t kCefSwitch[] = JOIN(L, CEF_SWITCH);
static constexpr wchar_t kRuntimeSwitch[] = JOIN(L, RUNTIME_SWITCH);

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

template <size_t length>
LPWSTR GetArg(LPWSTR arg, const wchar_t(&kSwitch)[length]) {
	if (wcsncmp(kSwitch, arg, size(kSwitch) - 1))
		return nullptr;

	auto cand = arg + size(kSwitch) - 1;
	if (*cand != '=')
		return nullptr;
	return cand + 1;
}

std::wstring GetAppDir() {
	wchar_t buffer[2048];
	GetModuleFileNameW(nullptr, buffer, size(buffer));
	auto slash = wcsrchr(buffer, LAUNCHER_DIRSEP);
	if (!slash)
		slash = buffer;
	*slash = 0;
	return buffer;
}

auto GetEnv(LPCWSTR name) {
	auto size = GetEnvironmentVariableW(name, nullptr, 0);
	if (size) {
		auto buf = std::make_unique<wchar_t[]>(size);
		GetEnvironmentVariableW(name, buf.get(), size);
		return buf;
	}
	return std::unique_ptr<wchar_t[]>{};
}

auto TryLoad(const std::wstring& dll) {
	auto pos = dll.rfind(LAUNCHER_DIRSEP);
	if (pos != std::string::npos) {
		auto dir = dll.substr(0, pos);
		auto path = GetEnv(L"PATH");
		if (path) {
			dir.push_back(LAUNCHER_PATHSEP);
			dir += path.get();
		}
		SetEnvironmentVariableW(L"PATH", dir.c_str());
		++pos;
	}
	else pos = 0;

	auto handle = LoadLibraryW(dll.c_str());
	auto err = GetLastError();
	if (!handle)
		MessageBox(nullptr, (L"Could not load " + dll.substr(pos) + L"\nAborting").c_str(), L"Kiosk", MB_OK | MB_ICONERROR);
	else
		FreeLibrary(handle);

	return !!handle;
};

size_t BufferFor(LPCWSTR text) {
	size_t length = 0, extra = 0, spaces = 0;
	while (*text) {
		switch (*text) {
		case '\\': case '"':
			++extra;
			break;
		case ' ':
			++spaces;
		}
		++length;
		++text;
	}
	if (spaces) extra += 2;
	return length + extra;
}

void Append(std::vector<wchar_t>& in, LPCWSTR text) {
	auto has_space = wcschr(text, ' ') != nullptr;

	if (has_space)
		in.push_back('"');

	auto start = text;
	while (*text) {
		while (*text && *text != '\\' && *text != '"')
			in.push_back(*text++);

		if (!*text) continue;

		if (*text == '"') {
			++text;
			in.push_back('\\');
			in.push_back('"');
			continue;
		}

		auto here = text;
		while (*text == '\\') ++text;
		auto dist = text - here;
		if (dist == 1)
			in.push_back('\\');
		if (dist > 1 || *text == '"') {
			while (here != text) {
				in.push_back('\\');
				in.push_back('\\');
				++here;
			}
		}
	}

	if (has_space) {
		if (text[-1] == '\\' && text[-2] != '\\')
			in.push_back('\\');
		in.push_back('"');
	}
}

std::vector<wchar_t> BuildCommandLine(const std::wstring& process, int argc, wchar_t** argv) {
	std::vector<wchar_t> out;

	auto fname_pos = process.rfind(LAUNCHER_DIRSEP);
	if (fname_pos == std::string::npos)
		fname_pos = 0;
	else
		++fname_pos;

	size_t reserve = BufferFor(process.c_str() + fname_pos);
	for (int i = 1; i < argc; ++i) {
		if (GetArg(argv[i], kCefSwitch) ||
			GetArg(argv[i], kRuntimeSwitch))
			continue;
		reserve += 1 + BufferFor(argv[i]);
	}

	out.reserve(reserve + 1); // for nil
	Append(out, process.c_str() + fname_pos);
	for (int i = 1; i < argc; ++i) {
		if (GetArg(argv[i], kCefSwitch) ||
			GetArg(argv[i], kRuntimeSwitch))
			continue;

		out.push_back(' ');
		Append(out, argv[i]);
	}
	out.push_back(0);

	return out;
}

int APIENTRY wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	{
		int argc = 0;
		local<LPWSTR[]>::ptr argv{ CommandLineToArgvW(GetCommandLineW(), &argc) };

		LPWSTR cef_path = nullptr;
		LPWSTR runtime_path = nullptr;
		for (int i = 1; i < argc; ++i) {
			auto arg = argv[i];
			if (!cef_path) {
				auto cand = GetArg(arg, kCefSwitch);
				if (cand) {
					cef_path = cand;
					if (runtime_path) break;
					continue;
				}
			}

			if (!runtime_path) {
				auto cand = GetArg(arg, kRuntimeSwitch);
				if (cand) {
					runtime_path = cand;
					if (cef_path) break;
					continue;
				}
			}
		}

		if (!cef_path) {
#ifdef USE_CEF_ROOT
			auto cef_root = GetEnv(L"CEF_ROOT");
			if (cef_root) {
#ifdef NDEBUG
#define CONFIG L"Release"
#else
#define CONFIG L"Debug"
#endif
				std::wstring cefroot = cef_root.get();
				auto lib = TryLoad(cefroot + DIRSEPW CONFIG DIRSEPW L"libcef.dll");
				if (!lib)
					return 1;
			} else {
#endif
				auto lib = TryLoad(GetAppDir() + DIRSEPW LAUNCHER_DIR_LIBW DIRSEPW LAUNCHER_LIBCEFW);
				if (!lib)
					return 1;
#ifdef USE_CEF_ROOT
			}
#endif
		} else {
			auto lib = TryLoad(cef_path);
			if (!lib)
				return 1;
		}

		auto runtime = runtime_path ? runtime_path : GetAppDir() + DIRSEPW LAUNCHER_DIR_RTW DIRSEPW LAUNCHER_RUNTIMEW;
		auto exe = TryLoad(runtime);
		if (!exe)
			return 2;

		auto cmdline = BuildCommandLine(runtime, argc, argv.get());
		if (!CreateProcessW(runtime.c_str(), cmdline.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
			MessageBox(nullptr, (L"Could not load\n" + runtime + L"\n\nAborting").c_str(), L"Kiosk", MB_OK | MB_ICONERROR);
			return 1;
		}
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}
