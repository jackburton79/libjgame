#include <jgame/Platform.h>

#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

using namespace jgame;

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\n", msg); \
			sFailures++; \
		} \
	} while (0)

int
main()
{
	CHECK(!Platform::IsMobile(), "the tests run on a computer");

	// The folder for the user's files: made, and ending with a slash (in a temporary home, so as not
	// to leave anything in the real one)
	char home[] = "/tmp/jgame-platform-XXXXXX";
	if (mkdtemp(home) == nullptr) {
		std::printf("mkdtemp failed\n");
		return EXIT_FAILURE;
	}
	setenv("HOME", home, 1);
	setenv("XDG_DATA_HOME", (std::string(home) + "/share").c_str(), 1);
	const std::string dir = Platform::UserDataDirectory("", "jgame-test");
	CHECK(!dir.empty(), "a folder");
	CHECK(!dir.empty() && dir.back() == '/', "ending with a slash");
	CHECK(dir.find("jgame-test") != std::string::npos, "named after the application");
	struct stat info;
	CHECK(stat(dir.c_str(), &info) == 0 && S_ISDIR(info.st_mode), "and made");
	const std::string withOrg = Platform::UserDataDirectory("someone", "jgame-test");
	CHECK(!withOrg.empty(), "with an organization too");

	if (sFailures == 0)
		std::printf("PlatformTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
