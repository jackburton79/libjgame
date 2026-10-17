// WriteAtomic() writes the whole file or leaves the old one alone, and never
// leaves its temporary file behind.
#include <jgame/FileStream.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>

using namespace jgame;

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\\n", msg); \
			sFailures++; \
		} \
	} while (0)

static bool
Exists(const std::string& path)
{
	return ::access(path.c_str(), F_OK) == 0;
}


static std::vector<char>
Contents(const std::string& path)
{
	std::vector<char> bytes;
	FILE* file = ::fopen(path.c_str(), "rb");
	if (file == NULL)
		return bytes;
	int c;
	while ((c = ::fgetc(file)) != EOF)
		bytes.push_back((char)c);
	::fclose(file);
	return bytes;
}


int
main()
{
	char pattern[] = "/tmp/libjgame-writeatomic-XXXXXX";
	const char* made = ::mkdtemp(pattern);
	if (made == NULL) {
		std::printf("FAIL: cannot make a temporary folder\n");
		return EXIT_FAILURE;
	}
	const std::string dir = made;
	const std::string path = dir + "/save.dat";

	const std::vector<char> first = { 'o', 'n', 'e', 0, 1, 2 };
	CHECK(FileStream::WriteAtomic(path.c_str(), first.data(), first.size()), "a new file was not written");
	CHECK(Contents(path) == first, "the new file has the wrong contents");
	CHECK(!Exists(path + ".tmp"), "the temporary file was left behind");

	// A shorter one takes its place, whole
	const std::vector<char> second = { 't', 'w' };
	CHECK(FileStream::WriteAtomic(path.c_str(), second.data(), second.size()), "the file was not replaced");
	CHECK(Contents(path) == second, "the replaced file has the wrong contents");

	// Nothing at all is a file too
	const std::string empty = dir + "/empty.dat";
	CHECK(FileStream::WriteAtomic(empty.c_str(), second.data(), 0), "an empty file was not written");
	CHECK(Exists(empty) && Contents(empty).empty(), "the empty file is not empty");

	// Where nothing can be written it says so, and the old file stays
	const std::string nowhere = dir + "/no/such/folder/save.dat";
	CHECK(!FileStream::WriteAtomic(nowhere.c_str(), first.data(), first.size()), "writing to a missing folder worked");
	CHECK(Contents(path) == second, "a failed write changed another file");

	// A folder is in the way: the data is written but can't take its place
	const std::string blocked = dir + "/blocked";
	::mkdir(blocked.c_str(), 0755);
	const std::string inside = blocked + "/keep";
	FileStream::WriteAtomic(inside.c_str(), first.data(), first.size());
	CHECK(!FileStream::WriteAtomic(blocked.c_str(), first.data(), first.size()), "a file took the place of a folder");
	CHECK(!Exists(blocked + ".tmp"), "a failed write left its temporary file behind");
	CHECK(Contents(inside) == first, "a failed write damaged what was there");

	::remove(inside.c_str());
	::rmdir(blocked.c_str());
	::remove(empty.c_str());
	::remove(path.c_str());
	::rmdir(dir.c_str());

	if (sFailures == 0)
		std::printf("WriteAtomicTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
