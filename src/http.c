#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "http.h"
#include "types.h"

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

	struct http_response *response = malloc(sizeof(struct http_response));
	memset(response, 0, sizeof(struct http_response));

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_secs);

	char header_buf[256];
	struct curl_slist *list = NULL;
	struct string_pair *current = headers;
	while (current != NULL) {
		memset(header_buf, 0, 256);
		snprintf((char *)header_buf, 255, "%s: %s", current->first,
			 current->second);

		list = curl_slist_append(list, (char *)header_buf);

		current = current->next;
	}
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

	struct http_response *response = malloc(sizeof(struct http_response));
	memset(response, 0, sizeof(struct http_response));

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_secs);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

	char header_buf[256];
	struct curl_slist *list = NULL;
	struct string_pair *current = headers;
	while (current != NULL) {
		memset(header_buf, 0, 256);
		snprintf((char *)header_buf, 255, "%s: %s", current->first,
			 current->second);

		list = curl_slist_append(list, (char *)header_buf);

		current = current->next;
	}
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

	char header_buf[256];
	struct curl_slist *list = NULL;
	struct string_pair *current = headers;
	while (current != NULL) {
		memset(header_buf, 0, 256);
		snprintf((char *)header_buf, 255, "%s: %s", current->first,
			 current->second);

		list = curl_slist_append(list, (char *)header_buf);

		current = current->next;
	}
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

#ifdef __cplusplus
}
#endif
