#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "http.h"
#include "types.h"

static struct curl_slist *build_headers(struct string_pair *headers) {
	char buf[256];
	struct curl_slist *list = NULL;
	for (struct string_pair *p = headers; p != NULL; p = p->next) {
		/* Silently truncates a header exceeding sizeof(buf); fine for
		 * this program's headers, but worth knowing. */
		snprintf(buf, sizeof(buf), "%s: %s", p->first, p->second);
		list = curl_slist_append(list, buf);
	}
	return list;
}

static size_t cb(void *data, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct http_response *r = (struct http_response *)userp;

	char *ptr = realloc(r->body, r->size + realsize + 1);
	if (ptr == NULL)
		return 0; /* out of memory! */

	r->body = ptr;
	memcpy(&(r->body[r->size]), data, realsize);
	r->size += realsize;
	r->body[r->size] = 0;

	return realsize;
}

struct http_response *http_get(const char *url, struct string_pair *headers,
			       long timeout_secs) {

	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		fputs("error: http_get: curl_easy_init\n", stderr);

		return NULL;
	}

	struct http_response *response = calloc(1, sizeof(struct http_response));
	if (response == NULL) {
		curl_easy_cleanup(curl);
		return NULL;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_secs);

	struct curl_slist *list = build_headers(headers);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

	CURLcode res = curl_easy_perform(curl);
	curl_slist_free_all(list);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));

		curl_easy_cleanup(curl);
		return NULL;
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
			  &(response->status_code));

	curl_easy_cleanup(curl);

	return response;
}

struct http_response *http_post(const char *url, struct string_pair *headers,
				const char *body, long timeout_secs) {

	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		fputs("error: http_post_no_response: curl_easy_init\n", stderr);

		return NULL;
	}

	struct http_response *response = calloc(1, sizeof(struct http_response));
	if (response == NULL) {
		curl_easy_cleanup(curl);
		return NULL;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_secs);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

	struct curl_slist *list = build_headers(headers);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

	CURLcode res = curl_easy_perform(curl);
	curl_slist_free_all(list);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));

		curl_easy_cleanup(curl);
		return NULL;
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
			  &(response->status_code));

	curl_easy_cleanup(curl);

	return response;
}

int http_post_no_response(const char *url, struct string_pair *headers,
			  const char *body, long timeout_secs) {

	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		fputs("error: http_post_no_response: curl_easy_init\n", stderr);

		return 1;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_secs);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

	struct curl_slist *list = build_headers(headers);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

	CURLcode res = curl_easy_perform(curl);
	curl_slist_free_all(list);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));

		curl_easy_cleanup(curl);
		return 1;
	}

	curl_easy_cleanup(curl);
	return 0;
}

void http_response_free(struct http_response *r) {
	if (r == NULL)
		return;

	string_pair_free_cascade(r->headers);

	if (r->body != NULL)
		free(r->body);

	free(r);
}
