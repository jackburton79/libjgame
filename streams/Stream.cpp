#include "Stream.h"

#include <cassert>
#include <iostream>
#include <stdexcept>


class SubStreamAdapter : public Stream {
public:
	SubStreamAdapter(Stream* stream, off_t offset, size_t length)
		:
		fParent(stream),
		fParentOffset(offset),
		fPosition(0),
		fSize(length)
	{
		assert(fParentOffset >= 0);
		assert(size_t(fParentOffset) < fParent->Size());
		assert(fSize <= fParent->Size() - fParentOffset);
		// TODO: The parent stream should be aware,
		// otherwise deleting it could cause leaks or memory corruption
	};
	virtual ssize_t ReadAt(off_t pos, void *dst, size_t size)
	{
		if (pos < 0 || (size_t)pos >= fSize)
			return -1;
		size_t readable = fSize - pos;
		if (size > readable)
			size = readable;
		return fParent->ReadAt(fParentOffset + pos, dst, size);
	};
	virtual off_t Seek(off_t where, int whence)
	{
		switch (whence) {
			case SEEK_SET:
				fPosition = where;
				break;
			case SEEK_CUR:
				fPosition += where;
				break;
			case SEEK_END:
				fPosition = fSize + where;
				break;
			default:
				break;
		}
		return fPosition;
	};
	virtual off_t Position() const
	{
		return fPosition;
	};

	virtual size_t Size() const
	{
		return fSize;
	}
private:
	Stream* fParent;
	off_t fParentOffset;
	off_t fPosition;
	size_t fSize;
};


// Stream
Stream::Stream()
{
}


Stream::~Stream()
{
}

	
ssize_t
Stream::Read(void *dst, size_t size)
{
	off_t curPos = Position();
	ssize_t read = ReadAt(curPos, dst, size);
	if (read > 0)
		Seek(read, SEEK_CUR);
	return read;
}


char*
Stream::ReadLine(char *buffer, size_t maxSize, char endLine)
{
	maxSize--;

	char *ptr = buffer;
	try {
		while ((*ptr = ReadByte()) != endLine
				&& (size_t)(ptr - buffer) < maxSize) {
			ptr++;
		}
	} catch (std::exception& e) {
		// eof
	}

	if (ptr > buffer) {
		*(ptr - 1) = '\0';
		return buffer;
	}

	return NULL;
}


ssize_t
Stream::Write(const void *src, size_t size)
{
	off_t curPos = Position();
	ssize_t wrote = WriteAt(curPos, src, size);
	if (wrote > 0)
		Seek(wrote, SEEK_CUR);
	return wrote;
}


uint8
Stream::ReadByte()
{
	uint8 byte;
	ssize_t read = Read(&byte, sizeof(byte));
	if (read != sizeof(byte))
		throw std::runtime_error("Stream::ReadByte(): tried to read uint8 but got less !");
	return byte;
}


uint16
Stream::ReadWordBE()
{
	uint8 result1 = ReadByte();
	uint8 result2 = ReadByte();

	return (uint16)((result1 << 8) | (result2));
}


uint16
Stream::ReadWordLE()
{
	uint16 result;
	if (Read(&result, sizeof(result)) != sizeof(result))
		throw std::runtime_error("ReadWordLE() read error");
	return result;
}


uint32
Stream::ReadDWordBE()
{
	uint16 result1 = ReadWordBE();
	uint16 result2 = ReadWordBE();

	return (uint32)((result1 << 16) | (result2));
}


uint32
Stream::ReadDWordLE()
{
	uint32 result;
	if (Read(&result, sizeof(result)) != sizeof(result))
		throw std::runtime_error("ReadDWordLE() read error");
	return result;
}


uint8
Stream::ReadByteAt(off_t offset)
{
	uint8 byte;
	ssize_t read = ReadAt(offset, &byte, sizeof(byte));
	if (read != sizeof(byte))
		throw std::runtime_error("Stream::ReadByteAt(): tried to read uint8 but got less !");
	return byte;
}


uint16
Stream::ReadWordBEAt(off_t offset)
{
	uint8 result1 = ReadByteAt(offset);
	uint8 result2 = ReadByteAt(offset + 1);

	return (uint16)((result1 << 8) | (result2));
}


uint16
Stream::ReadWordLEAt(off_t offset)
{
	uint16 result;
	if (ReadAt(offset, &result, sizeof(result)) != sizeof(result))
		throw std::runtime_error("ReadWordLEAt() read error");
	return result;
}


uint32
Stream::ReadDWordBEAt(off_t offset)
{
	uint16 result1 = ReadWordBEAt(offset);
	uint16 result2 = ReadWordBEAt(offset + sizeof(uint16));

	return (uint32)((result1 << 16) | (result2));
}


uint32
Stream::ReadDWordLEAt(off_t offset)
{
	uint32 result;
	if (ReadAt(offset, &result, sizeof(result)) != sizeof(result))
		throw std::runtime_error("ReadDWordLEAt() read error");
	return result;
}


/* virtual */
Stream*
Stream::Clone() const
{
	std::cerr << "Stream::Clone() not implemented!";
	throw std::runtime_error("Stream::Clone() not implemented!");
	return NULL;
}


/* virtual */
Stream*
Stream::Adopt()
{
	std::cerr << "Stream::Adopt() not implemented!";
	throw std::runtime_error("Stream::Adopt() not implemented!");
	return NULL;
}


Stream*
Stream::SubStream(off_t offset, size_t size)
{
	return new SubStreamAdapter(this, offset, size);
}


void
Stream::Dump()
{
	off_t oldPos = Position();
	Seek(0, SEEK_SET);
	uint8 byte;
	try {
		while (true) {
			byte = ReadByte();
			std::cout << byte;
		}
	} catch (std::exception& e) {
		// eof
	}

	Seek(oldPos, SEEK_SET);
}


void
Stream::DumpToFile(const char *fileName)
{
	FILE *file = fopen(fileName, "wb");
	if (file) {
		off_t oldPos = Position();
		Seek(0, SEEK_SET);
		ssize_t read;
		char buffer[1024];
		while ((read = Read(buffer, 1024)) > 0)
			fwrite(buffer, read, 1, file);
		fclose(file);
		Seek(oldPos, SEEK_SET);
	}
}
