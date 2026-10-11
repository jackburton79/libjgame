// Numbers go into a buffer low byte first, come back the same, and the
// checksum notices a changed byte.
#include "Bytes.h"

#include <cstdio>
#include <cstdlib>

static int sFailures = 0;

#define CHECK(cond, msg) \
	do { \
		if (!(cond)) { \
			std::printf("FAIL: %s\\n", msg); \
			sFailures++; \
		} \
	} while (0)

int
main()
{
	std::vector<uint8_t> buffer;
	Bytes::Put(buffer, 0x1234, 2);
	Bytes::Put(buffer, 0xA1B2C3D4u, 4);
	Bytes::Put(buffer, 0x0102030405060708ull, 8);
	CHECK(buffer.size() == 14, "2 + 4 + 8 bytes are not 14");
	CHECK(buffer[0] == 0x34 && buffer[1] == 0x12, "the low byte doesn't come first");
	CHECK(buffer[2] == 0xD4 && buffer[5] == 0xA1, "a 4 byte number is in the wrong order");

	CHECK(Bytes::Get(buffer.data(), 2) == 0x1234, "a 2 byte number came back different");
	CHECK(Bytes::Get(buffer.data() + 2, 4) == 0xA1B2C3D4u, "a 4 byte number came back different");
	CHECK(Bytes::Get(buffer.data() + 6, 8) == 0x0102030405060708ull, "an 8 byte number came back different");

	// Only as many bytes as asked for
	std::vector<uint8_t> cut;
	Bytes::Put(cut, 0x12345678, 1);
	CHECK(cut.size() == 1 && cut[0] == 0x78, "a number cut to one byte");

	// FNV-1a reference values
	CHECK(Bytes::Checksum(nullptr, 0) == 0x811C9DC5u, "the checksum of nothing");
	const uint8_t a[] = { 'a' };
	CHECK(Bytes::Checksum(a, 1) == 0xE40C292Cu, "the checksum of 'a'");
	const uint8_t foobar[] = { 'f', 'o', 'o', 'b', 'a', 'r' };
	CHECK(Bytes::Checksum(foobar, 6) == 0xBF9CF968u, "the checksum of 'foobar'");

	const uint32_t before = Bytes::Checksum(buffer.data(), buffer.size());
	buffer[7] ^= 1;
	CHECK(Bytes::Checksum(buffer.data(), buffer.size()) != before, "the checksum missed a changed bit");

	if (sFailures == 0)
		std::printf("BytesTest: all good\n");
	return sFailures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
