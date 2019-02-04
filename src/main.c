#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <libxml/parser.h>

#include "http.h"
#include "strfind.h"

#define MAXURL 512

static inline char *xml_text(xmlDoc *doc, xmlNode *node) {
	return (char *)xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
}

void print_item(int *index, char *title, int cols) {
	printf("%2d ", *index);
	int avail_cols = cols - 3; // "%2d " prefix
	int len = strlen(title);
	if (len < avail_cols) {
		puts(title);
	} else {
		int trunclen = avail_cols - 4; // "... " suffix
		char *c = title;
		for (int i = 0; c && i < trunclen; i += 1)
			putchar(*c++);
		puts("... ");
	}
}

void grab_item(char *path, int cols, xmlDoc *doc, xmlNode *node, int *index) {
	if (strcmp(((char *)node->name), "item"))
		return;

	char *title = NULL;
	char *url = NULL;

	*index += 1;
	for (xmlNode *attr = node->children; attr; attr = attr->next) {
		if (!title && !strcmp((char *)attr->name, "title"))
			title = xml_text(doc, attr);

		if (!url && !strcmp((char *)attr->name, "comments"))
			url = xml_text(doc, attr);

		if (title && url)
			break;
	}

	if (!title || !url) {
		fputs("error: grab_item\n", stderr);
		if (title)
			free(title);
		if (url)
			free(url);
		return;
	}

	FILE *fd = fopen(path, "a");
	fprintf(fd, "%s\n", url);
	fclose(fd);

	print_item(index, title, cols);
	free(title);
	free(url);
}

int last_index(const char *string, const char c) {
	size_t len = strlen(string);

	for (int i = len - 1; i >= 0; i -= 1)
		if (string[i] == c)
			return i;

	return -1;
}

int fetch_feed(char *path) {
	struct winsize termsize;
	ioctl(0, TIOCGWINSZ, (char *)&termsize);

	struct string_pair *host =
	    string_pair_new("Host", "news.ycombinator.com");
	struct string_pair *agent = string_pair_new("User-Agent", "curl/0.0.1");
	host->next = agent;

	struct http_response *response =
	    http_get("https://news.ycombinator.com/rss", NULL, 5);

	if (response == NULL) {
		fputs("error: http_get\n", stderr);
		return 1;
	}

	char *xml_part = strchr(response->body, '<');
	int xml_end_idx = last_index(xml_part, '>');
	if (xml_end_idx < 0) {
		fputs("error: did not get XML response\n", stderr);
		return 1;
	}
	size_t xml_len = xml_end_idx + 1;
	xml_part[xml_end_idx + 1] = '\0';

	xmlDoc *doc = xmlReadMemory(xml_part, xml_len, NULL, NULL, 0);

	free(response);
	string_pair_free_cascade(host);

	xmlNode *root = xmlDocGetRootElement(doc);
	xmlNode *channel = root->children;

	int index = 0;
	for (xmlNode *node = channel->children; node; node = node->next) {
		grab_item(path, termsize.ws_col, doc, node, &index);
		if (index == (termsize.ws_row - 1))
			break; // Print only enough to fill the screen.
	}

	xmlFreeDoc(doc);

	return 0;
}

void open_item(char *path, unsigned long target) {
	char url[MAXURL];

	FILE *fd = fopen(path, "r");
	for (unsigned long i = 1; i <= target; i += 1) {
		fgets(url, MAXURL, fd);
		if (feof(fd)) {
			fprintf(stderr, "invalid index: %lu\n", target);
			fclose(fd);
			return;
		}
	}
	fclose(fd);

#if defined(__APPLE__)
	char *cmd;
	asprintf(&cmd, "open %s", url);
	system(cmd);
	free(cmd);
#elif defined(__linux__)
	char *cmd;
	asprintf(&cmd, "xdg-open %s", url);
	system(cmd);
	free(cmd);
#else
	fputs("OS not supported\n", stderr);
	exit(1);
#endif
}

int main(int argc, char **argv) {
	char *path;
	asprintf(&path, "%s/.hn", getenv("HOME"));

	if (argc == 1) {
		remove(path);
		int rc = fetch_feed(path);
		free(path);
		return rc;
	}

	if (access(path, F_OK | R_OK) == -1) {
		fputs("index file does not exist\n", stderr);
		free(path);
		return 1;
	}

	for (int i = 0; i < (argc - 1); i += 1) {
		errno = 0;
		unsigned long index = strtoul(argv[i + 1], NULL, 10);

		if (errno == EINVAL || errno == ERANGE) {
			fprintf(stderr, "invalid index: %s\n", argv[i + 1]);

			continue;
		}

		open_item(path, index);

		if (argc > 2 && i > 0) {
			puts("Sleeping.");
			usleep(200 * 1000);
		}
	}

	free(path);
}
