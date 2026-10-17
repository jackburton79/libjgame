// MemoryStream::Adopt(): the new stream takes over the data, and frees it only
// if the old one would have.
#include "MemoryStream.h"

#include <cstdio>
#include <cstdlib>

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
	// Not the stream's: deleting the adopted stream must leave it alone
	// (freeing it would be a delete[] on the stack)
	uint8 notOwned[4] = { 1, 2, 3, 4 };
	MemoryStream* stream = new MemoryStream(notOwned, sizeof(notOwned), false);
	MemoryStream* adopted = stream->Adopt();
	CHECK(adopted->Size() == sizeof(notOwned), "the adopted stream has the wrong size");
	CHECK(adopted->ReadByteAt(2) == 3, "the adopted stream reads the wrong data");
	CHECK(stream->Size() == 0, "the old stream still has the data");
	delete stream;
	delete adopted;
	CHECK(notOwned[0] == 1 && notOwned[3] == 4, "the data was touched");

	// The stream's own: the adopted one frees it, the old one doesn't
	uint8* owned = new uint8[3] { 7, 8, 9 };
	stream = new MemoryStream(owned, 3, true);
	adopted = stream->Adopt();
	delete stream;
	CHECK(adopted->ReadByteAt(0) == 7, "the data went with the old stream");
	delete adopted;

	if (sFailures == 0)
		std::printf("MemoryStreamTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
