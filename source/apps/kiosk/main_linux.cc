#include <dlfcn.h>
#include <memory>
#include <paths.h>
#include <string>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

static constexpr char kCefSwitch[] = CEF_SWITCH;
static constexpr char kRuntimeSwitch[] = RUNTIME_SWITCH;

template <size_t length>
const char* GetArg(const char* arg, const char(&kSwitch)[length]) {
	if (strncmp(kSwitch, arg, size(kSwitch) - 1))
		return nullptr;

	auto cand = arg + size(kSwitch) - 1;
	if (*cand != '=')
		return nullptr;
	return cand + 1;
}

struct so {
	struct closer {
		void operator()(void* dl) {
			dlclose(dl);
		}
	};

	using handle = std::unique_ptr<void, closer>;
};

so::handle TryLoad(const std::string& module) {
	auto pos = module.rfind(LAUNCHER_DIRSEP);
	if (pos != std::string::npos) {
		auto dir = module.substr(0, pos);
		auto path = getenv("LD_LIBRARY_PATH");
		if (path) {
			dir.push_back(LAUNCHER_PATHSEP);
			dir += path;
		}
		setenv("LD_LIBRARY_PATH", dir.c_str(), 1);
		++pos;
	}
	else pos = 0;

	so::handle out { dlopen(module.c_str(), RTLD_NOW) };
	if (!out) {
		fprintf(stderr, "kiosk: %s\n", dlerror());
	}

	return out;
}

std::string FindRuntime(int argc, char* argv[]) {
	const char* cef_path = nullptr;
	const char* runtime_path = nullptr;
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
		auto cef_root = getenv("CEF_ROOT");
		if (cef_root) {
#ifdef NDEBUG
#define CONFIG "Release"
#else
#define CONFIG "Debug"
#endif
			std::string cefroot = cef_root;
			auto lib = TryLoad(cefroot + LAUNCHER_DIRSTR CONFIG LAUNCHER_DIRSTR LAUNCHER_LIBCEF);
			if (!lib)
				return {};
		} else {
#endif
			auto lib = TryLoad(LAUNCHER_DIR_LIB LAUNCHER_DIRSTR LAUNCHER_LIBCEF);
			if (!lib)
				return {};
#ifdef USE_CEF_ROOT
		}
#endif
	} else {
		auto lib = TryLoad(cef_path);
		if (!lib)
			return {};
	}

	return runtime_path ? runtime_path : LAUNCHER_DIR_RT LAUNCHER_DIRSTR LAUNCHER_RUNTIME;
}

int main(int argc, char* argv[])
{
	pid_t child = 0;

	{
		auto runtime = FindRuntime(argc, argv);
		if (runtime.empty())
			return 1;

		child = fork();
		if (!child) {
			std::vector<char*> args(argc + 1);
			args[0] = &runtime[0];
			int j = 0;
			for (int i = 1; i < argc; ++i) {
				if (GetArg(argv[i], kCefSwitch) ||
					GetArg(argv[i], kRuntimeSwitch))
					continue;
				args[j++] = argv[i];
			}
			args[j] = nullptr;
			execvp(runtime.c_str(), args.data());
			int err = errno;
			fprintf(stderr, "kiosk: %s: %s\n", runtime.c_str(), strerror(err));
			_exit(2);
		}
	}

	int stat = 0;
	wait(&stat);

	if (WIFEXITED(stat))
		return WEXITSTATUS(stat);
	return 2;
}
