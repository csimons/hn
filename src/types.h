#ifndef BERT_TYPES_H
#define BERT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

struct string_pair {
	char *first;
	char *second;
	struct string_pair *next;
};

void string_pair_free(struct string_pair *h);
void string_pair_free_cascade(struct string_pair *h);
struct string_pair *string_pair_new(const char *first, const char *second);

#ifdef __cplusplus
}
#endif

#endif
