// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "types.h"

static u32 fnv1a_32(const char *str)
{
	u32 prime = 0x01000193;
	u32 offset = 0x811c9dc5;

	u32 hash = offset;
	while(*str)
	{
		hash ^= *str;
		hash *= prime;
		++str;
	}
	return hash;
}

static u64 fnv1a_64(const char *str)
{
	u64 prime = 0x00000100000001B3;
	u64 offset = 0xcbf29ce484222325;

	u64 hash = offset;
	while(*str)
	{
		hash ^= *str;
		hash *= prime;
		++str;
	}
	return hash;
}

static void print_hex_string(char *data, size_t n)
{
	for(size_t i = 0; i < n; ++i)
	{
		printf("%02X", data[n - i - 1] & 0xff);
	}
}