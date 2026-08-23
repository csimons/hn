#include <stdlib.h>
#include <string.h>

#include "types.h"

void string_pair_free(struct string_pair *h) {
	if (h == NULL)
		return;

	if (h->first != NULL)
		free(h->first);

	if (h->second != NULL)
		free(h->second);

	free(h);
}

void string_pair_free_cascade(struct string_pair *h) {
	while (h != NULL) {
		struct string_pair *next = h->next;
		string_pair_free(h);
		h = next;
	}
}

struct string_pair *string_pair_new(const char *first, const char *second) {
	struct string_pair *h = calloc(1, sizeof(struct string_pair));
	if (h == NULL)
		return NULL;

	h->first = strdup(first);
	h->second = strdup(second);
	if (h->first == NULL || h->second == NULL) {
		string_pair_free(h);
		return NULL;
	}

	return h;
}
