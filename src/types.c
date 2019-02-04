#ifdef __cplusplus
extern "C" {
#endif

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
	if (h == NULL)
		return;

	struct string_pair *tmp = NULL;
	if (h->next != NULL)
		tmp = h->next;

	string_pair_free(h);
	if (tmp != NULL)
		string_pair_free_cascade(tmp);
}

struct string_pair *string_pair_new(const char *first, const char *second) {
	struct string_pair *h = malloc(sizeof(struct string_pair));
	memset(h, 0, sizeof(struct string_pair));
	h->first = strdup(first);
	h->second = strdup(second);
	h->next = NULL;
	return h;
}

#ifdef __cplusplus
}
#endif
