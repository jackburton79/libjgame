#include <cstring>
#include <iostream>

#include "Utils.h"

void check(const char* input, const char* expected)
{
	std::string actual = trimmed(input);

	if (std::strcmp(actual.c_str(), expected) != 0)
	{
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

int main()
{
	check("", "");
	check("Hello", "Hello");
	check("Hello ", "Hello");
	check("Hello   ", "Hello");
	check("   Hello", "   Hello");
	check("   Hello   ", "   Hello");
	check(" ", "");
	check("     ", "");
	check("Hello   World", "Hello   World");
	check("Hello   World   ", "Hello   World");

	return 0;
}
