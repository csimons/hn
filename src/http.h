#ifndef BERT_HTTP_H
#define BERT_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "types.h"

struct http_response {
	long status_code;
	size_t size;
	struct string_pair *headers;
	char *body;
};

/* Returns allocated memory, or NULL for failure. */
struct http_response *http_get(const char *url, struct string_pair *headers,
			       long timeout_secs);

/* Returns allocated memory, or NULL for failure. */
struct http_response *http_post(const char *url, struct string_pair *headers,
				const char *body, long timeout_secs);

/* Returns 0 on success, 1 on failure. */
int http_post_no_response(const char *url, struct string_pair *headers,
			  const char *body, long timeout_secs);

void http_response_free(struct http_response *r);

#ifdef __cplusplus
}
#endif

#endif
