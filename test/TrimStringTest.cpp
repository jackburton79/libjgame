#include <cstring>
#include <iostream>
#include <vector>

#include "Utils.h"

static int sFailures = 0;

static void report(const char* input, const char* expected, const char* actual)
{
	if (std::strcmp(actual, expected) != 0)
	{
		sFailures++;
		std::cout
			<< "FAIL\n"
			<< "Input    : [" << input << "]\n"
			<< "Expected : [" << expected << "]\n"
			<< "Actual   : [" << actual << "]\n\n";
	}
	else
	{
		std::cout << "PASS [" << input << "]\n";
	}
}

// Both: trimmed() copies, trim() works in place (an empty or all-blank string
// must not make it read before the start of the buffer)
void check(const char* input, const char* expected)
{
	report(input, expected, trimmed(input).c_str());

	// Exactly as big as the string, so a read before or after it shows up
	// under a sanitizer
	std::vector<char> buffer(input, input + std::strlen(input) + 1);
	report(input, expected, trim(buffer.data()));
}

int main()
{
	check("", "");
	check("Hello", "Hello");
	check("Hello ", "Hello");
	check("Hello   ", "Hello");
	check("   Hello", "Hello");
	check("   Hello   ", "Hello");
	check(" ", "");
	check("     ", "");
	check("Hello   World", "Hello   World");
	check("Hello   World   ", "Hello   World");
	check("\t Hello \n", "Hello");

	return sFailures == 0 ? 0 : 1;
}
