#ifndef __MEMORY_H
#define __MEMORY_H

#include "Stream.h"
#include "SupportDefs.h"

class MemoryStream : public Stream {
public:
	MemoryStream(const uint8 *data, int size, bool owns = false);
	MemoryStream(int size); // Allocates a block of memory
	
	virtual ~MemoryStream();
	
	virtual ssize_t Read(void *dst, int size);
	virtual ssize_t ReadAt(int pos, void *dst, int size);
	virtual ssize_t WriteAt(int pos, const void *src, int size);
	
	virtual int32 Seek(int32 where, int whence);
	virtual int32 Position() const;
	
	virtual uint32 Size() const;
	
	virtual void *Data() const;
	
public:
	uint8 *fData;
	int fSize;
	int32 fPosition;
	bool fOwnsBuffer; 
};

#endif
