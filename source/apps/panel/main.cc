#include <version.h>
#include <cstdio>

int main()
{
	std::printf("%s v%s\n", PROGRAM_NAME, VERSION_FULL);
}
