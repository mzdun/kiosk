#include <dlfcn.h>
#include <memory>
#include <paths.h>
#include <string>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "main_kiosk.h"
#include "../common/switches.h"

static constexpr char kCefSwitch[] = "--" CEF_SWITCH;
static constexpr char kRuntimeSwitch[] = "--" RUNTIME_SWITCH;
static constexpr char kRootSwitch[] = "--" ROOT_SWITCH;

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

bool TryLoad(const std::string& module) {
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

	return !!out;
}

int main(int argc, char* argv[])
{
	pid_t child = 0;

	{
		auto runtime = FindRuntime(argc, argv, kCefSwitch, kRuntimeSwitch, kRootSwitch);
		if (runtime.empty())
			return 1;

		child = fork();
		if (!child) {
			std::vector<char*> args(argc + 1);
			args[0] = &runtime[0];
			int j = 1;
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
