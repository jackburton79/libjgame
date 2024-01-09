#include "FileStream.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <dirent.h>

#include "Utils.h"


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
		fFileHandle = ::fopen_case(filename, flags);
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

