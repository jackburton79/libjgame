#include "Utils.h"


#include <algorithm> 
#include <assert.h>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <string>

#include "Path.h"


// Source - https://stackoverflow.com/a/217605
// Posted by Evan Teran, modified by community. See post 'Timeline' for change history
// Retrieved 2026-09-16, License - CC BY-SA 4.0


// Trim from the start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trim from the end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Source - https://stackoverflow.com/a/217605
// Posted by Evan Teran, modified by community. See post 'Timeline' for change history
// Retrieved 2026-09-16, License - CC BY-SA 4.0

// Trim from both ends (in place)
char*
trim(char* string)
{
	char* newStringStart = string;
	while (isspace(*newStringStart))
		newStringStart++;
 
	char* endOfString = newStringStart + ::strlen(newStringStart) - 1;
	while (isspace(*endOfString))
		endOfString--;
 
	endOfString++;
	*endOfString  = '\0';

	return newStringStart;
}


// Trim from the start (copying)
std::string
ltrimmed(std::string string)
{
    ltrim(string);
    return string;
}


// Trim from the end (copying)
std::string
rtrimmed(std::string string)
{
    rtrim(string);
    return string;
}


// Trim from both ends (copying)
std::string
trimmed(const char *string)
{
	std::string copiedString(string);
    ltrim(copiedString);
	rtrim(copiedString);
    return copiedString;
}


void
path_dos_to_unix(char* path)
{
	char* c;
	while ((c = ::strchr(path, '\\')) != NULL)
		*c = '/';
}


FILE*
fopen_case(const char* filename, const char* flags)
{
	assert(filename != NULL);
	assert(::strlen(filename) > 1);

	Storage::Path normalizedFileName(filename);
	std::string newPath("/");
	char* start = const_cast<char*>(normalizedFileName.String()) + 1;
	char* end = start + ::strlen(normalizedFileName.String());
	size_t where = 0;
	while ((where = ::strcspn(start, "/")) > 0) {
		std::string leaf;
		leaf.append(start, where);
		DIR* dir = ::opendir(newPath.c_str());
		if (dir != NULL) {
			dirent *entry = NULL;
			while ((entry = ::readdir(dir)) != NULL) {
				if (!::strcasecmp(entry->d_name, leaf.c_str())) {
					if (newPath != "/")
						newPath.append("/");
					newPath.append(entry->d_name);
					break;
				}
			}
			::closedir(dir);
		}
		start += where + 1;
		if (start >= end)
			break;
	}

	FILE* handle = NULL;
	if (!::strcasecmp(filename, newPath.c_str()))
		handle = ::fopen(newPath.c_str(), flags);

	return handle;
}


const char*
extension(const char* path)
{
	if (path == NULL)
		return NULL;

	const char* point = ::strrchr(path, '.');
	if (point == NULL)
		return NULL;

	return point + 1;
}


size_t
get_unquoted_string(char* dest, char* source, size_t size)
{
	char* name = source;
	char* nameEnd = name + size;
	while (*name == '"')
		name++;
	while (*nameEnd != '"')
		nameEnd--;
	*nameEnd = '\0';
	::strncpy(dest, name, size);

	return nameEnd - source;
}


bool
is_bit_set(uint32 value, int bitPos)
{
	return value & (1 << bitPos);
}


void
set_bit(uint32& value, int bitPos)
{
	value |= (1 << bitPos);
}


void
clear_bit(uint32& value, int bitPos)
{
	value &= ~(1 << bitPos);
}


void
assert_size(size_t size, size_t controlValue)
{
	if (size != controlValue) {
		printf("size is %zu\n", size);
		assert(size == controlValue);
	}
}
