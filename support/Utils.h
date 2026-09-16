#pragma once

#include <cstdio>
#include <string>

#include "SupportDefs.h"

// Trim in place
char* trim(char* string);

// Does not touch the passed string
std::string trimmed(const char* string);

// Replaces "\" to "/"
void path_dos_to_unix(char* path);

// Opens a file case insensitively on a case sensitive FS
FILE* fopen_case(const char* name, const char* flags);
const char* extension(const char* name);

size_t get_unquoted_string(char* dest, char* source, size_t maxSize);

// bit manipulation
bool is_bit_set(uint32 value, int bitPos);
void set_bit(uint32& value, int bitPos);
void clear_bit(uint32& value, int bitPos);

void assert_size(size_t size, size_t controlValue);
