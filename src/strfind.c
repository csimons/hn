#include <stdio.h>
#include <string.h>

int strfind(const char *haystack, const char *needle) {
	int result = -1;

	if (haystack == NULL || needle == NULL)
		return -1;

	size_t haystack_n = strlen(haystack);
	size_t needle_n = strlen(needle);

	if (needle_n == 0)
		return 0;

	if (needle_n > haystack_n)
		return -1;

	for (size_t start_idx = 0; start_idx < haystack_n; start_idx += 1) {
		size_t matching_chars = 0;
		for (size_t j = 0; j < needle_n; j += 1) {
			if (haystack[start_idx + j] == needle[j]) {
				matching_chars += 1;
				if (matching_chars == needle_n)
					return start_idx;
			} else {
				break;
			}
		}
	}

	return result;
}
