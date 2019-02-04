#ifndef SUBSTRING_H
#define SUBSTRING_H

#include <stddef.h>

/*
 * Returns an allocated subtring of the provided original string.
 * Stops at size (end - begin) or at the first null byte encountered,
 * whichever comes first..
 */
char *substring(const char *original, size_t begin, size_t end);

#endif
