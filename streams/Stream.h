#ifndef __STREAM_H
#define __STREAM_H

#include <cstdio>
#include <stdexcept>

#include <SupportDefs.h>

class Stream {
public:
	Stream();
	virtual ~Stream();
	
	virtual ssize_t Read(void *dst, size_t size);
	virtual ssize_t ReadAt(off_t pos, void *dst, size_t size) = 0;

	char *ReadLine(char *buffer, size_t maxSize, char endLine = '\n');

	bool Eof() const { return (size_t)Position() >= Size(); };

	virtual ssize_t Write(const void *src, size_t size);
	virtual ssize_t WriteAt(off_t pos, const void *src, size_t size)
	{
		throw std::runtime_error("Stream is not writable!");
	};
	
	virtual off_t Seek(off_t where, int whence)
	{
		throw std::runtime_error("Seek() not supported for this stream!");
	};
	
	virtual off_t Position() const 
	{
		throw std::runtime_error("Position() not supported for this stream!");
	};
	
	virtual size_t Size() const
	{
		throw std::runtime_error("Size() not supported for this stream!");
	}
	
	virtual void *Data() const 
	{
		throw std::runtime_error("Data() not supported for this stream!");
	};
	
	virtual uint8 ReadByte();
	uint8 ReadByteAt(off_t offset);

	// TODO: these only work on LE machines for now
	// Use these only if you need to swap endianess, because they are
	// not efficient: they read one byte at a time
	uint16 ReadWordLE();
	uint16 ReadWordBE();
	uint32 ReadDWordLE();
	uint32 ReadDWordBE();

	uint16 ReadWordLEAt(off_t offset);
	uint16 ReadWordBEAt(off_t offset);
	uint32 ReadDWordLEAt(off_t offset);
	uint32 ReadDWordBEAt(off_t offset);

	template<typename T>
	Stream& operator>>(T& data) {
		ssize_t read = Read(&data, sizeof(data));
		if (read < 0 || read != sizeof(data))
			throw std::runtime_error("Read() exception");
		return *this;
	};

	template<typename T>
	void Read(T& dest) {
		ssize_t read = Read(&dest, sizeof(dest));
		if (read < 0 || read != sizeof(dest))
			throw std::runtime_error("Read() exception");
	}
	
	template<typename T>
	void ReadAt(off_t pos, T &dst) {
		ssize_t read = ReadAt(pos, &dst, sizeof(dst));
		if (read < 0 || read != sizeof(dst))
			throw std::runtime_error("ReadAt() exception");
	};

	virtual Stream* Clone() const;
	virtual Stream* Adopt();
	Stream* SubStream(off_t pos, size_t size);

	void Dump();
	void DumpToFile(const char *fileName);
};


#endif
