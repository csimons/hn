#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "substring.h"

char *substring(const char *original, size_t begin, size_t end) {
	if (end <= begin)
		return NULL;

	size_t sz = (end - begin) + 1;

	char *segment = malloc(sz);
	memset(segment, 0, sz);

	for (size_t i = 0; i < (end - begin); i += 1) {
		if (original[begin + i] == 0)
			return segment;

		segment[i] = original[begin + i];
	}

	return segment;
}
