#include <jgame/FileStream.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <filesystem>

#include <jgame/Utils.h>

namespace jgame {

static const int ACCESS_MODE_MASK = 0x03;
static const int FLAGS_MASK = 0xFFF8;


FileStream::FileStream(const char *filename, int mode)
{		
	int status = SetTo(filename, mode);
	if (status != 0) {
		std::string error;
		error += ::strerror(status);
		error += " (\"";
		error += filename;
		error += "\")";
		throw std::runtime_error(error);
	}
}


FileStream::FileStream()
	:
	fFileHandle(NULL)
{
}


FileStream::~FileStream()
{
	if (fFileHandle != NULL)
		::fclose(fFileHandle);
}


int
FileStream::SetTo(const char *filename, int mode)
{
	const char *flags = NULL;
	
	int accessMode = mode & ACCESS_MODE_MASK;
	int openFlags = mode & FLAGS_MASK;
	
	switch (accessMode) {
		case READ_WRITE:
			flags = "wrb";	
			break;
		case WRITE_ONLY:
			flags = "wb";
			break;
		case READ_ONLY:
		default:
			flags = "rb";
			break;
	}
	
	if (openFlags & FileStream::IGNORE_CASE)
		fFileHandle = fopen_case(filename, flags);
	else
		fFileHandle = ::fopen(filename, flags);

	if (fFileHandle == NULL)
		return errno;

	return 0;
}


bool
FileStream::IsValid() const
{
	return fFileHandle != NULL;
}


off_t
FileStream::Seek(off_t where, int whence)
{
	::fseek(fFileHandle, where, whence);
	return ::ftell(fFileHandle);
}


off_t
FileStream::Position() const
{
	return ::ftell(fFileHandle);
}


size_t
FileStream::Size() const
{
	const off_t oldpos = Position();
	size_t fileSize = const_cast<FileStream*>(this)->Seek(0L, SEEK_END);

	const_cast<FileStream*>(this)->Seek(oldpos, SEEK_SET);

	return fileSize;
}


bool
FileStream::EoF()
{
	return ::feof(fFileHandle) != 0;
}


ssize_t
FileStream::ReadAt(off_t pos, void *dst, size_t size)
{
	off_t oldPos = ::ftell(fFileHandle);
	::fseek(fFileHandle, pos, SEEK_SET);
	ssize_t read = ::fread(dst, 1, size, fFileHandle);
	::fseek(fFileHandle, oldPos, SEEK_SET);

	return read;
}


ssize_t
FileStream::Read(void *dst, size_t count)
{
	return ::fread(dst, 1, count, fFileHandle);
}


ssize_t
FileStream::Write(const void *src, size_t count)
{
	return ::fwrite(src, 1, count, fFileHandle);
}


/* static */
bool
FileStream::WriteAtomic(const char *filename, const void *data, size_t size)
{
	std::string temp = filename;
	temp += ".tmp";

	FILE* file = ::fopen(temp.c_str(), "wb");
	if (file == NULL)
		return false;

	// What is written may still be in a buffer: closing is what can fail
	bool written = ::fwrite(data, 1, size, file) == size;
	if (::fclose(file) != 0)
		written = false;

	// (std::filesystem::rename() replaces a file that is there already on every system; the C rename()
	// refuses to on Windows, where saving over the last save would never work)
	std::error_code renameError;
	if (written)
		std::filesystem::rename(temp, filename, renameError);
	if (!written || renameError) {
		::remove(temp.c_str());
		return false;
	}
	return true;
}

} // namespace jgame
