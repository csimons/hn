#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <libxml/parser.h>

#include "http.h"

#define MAXURL 512
#define ITEM_PREFIX "%2d "
#define ITEM_PREFIX_LEN ((int)sizeof("NN ") - 1)
#define TRUNC_SUFFIX "..."
#define TRUNC_SUFFIX_LEN ((int)sizeof(TRUNC_SUFFIX) - 1)

static inline char *xml_text(xmlDoc *doc, xmlNode *node) {
	return (char *)xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
}

void print_item(int index, const char *title, int cols) {
	printf(ITEM_PREFIX, index);
	int avail_cols = cols - ITEM_PREFIX_LEN;
	int len = strlen(title);
	if (len < avail_cols) {
		puts(title);
	} else {
		int trunclen = avail_cols - TRUNC_SUFFIX_LEN;
		const char *c = title;
		for (int i = 0; c && i < trunclen; i += 1)
			putchar(*c++);
		puts(TRUNC_SUFFIX);
	}
}

void grab_item(FILE *urlfile, int cols, xmlDoc *doc, xmlNode *node, int *index) {
	if (strcmp(((char *)node->name), "item"))
		return;

	char *title = NULL;
	char *comments_url = NULL;

	*index += 1;
	for (xmlNode *attr = node->children; attr; attr = attr->next) {
		if (!title && !strcmp((char *)attr->name, "title"))
			title = xml_text(doc, attr);

		if (!comments_url && !strcmp((char *)attr->name, "comments"))
			comments_url = xml_text(doc, attr);

		if (title && comments_url)
			break;
	}

	if (!title || !comments_url) {
		fputs("error: grab_item\n", stderr);
		if (title)
			free(title);
		if (comments_url)
			free(comments_url);
		return;
	}

	fprintf(urlfile, "%s\n", comments_url);

	print_item(*index, title, cols);
	free(title);
	free(comments_url);
}

int fetch_feed(char *path) {
	struct winsize termsize;
	if (ioctl(0, TIOCGWINSZ, (char *)&termsize) == -1) {
		termsize.ws_row = 24;
		termsize.ws_col = 80;
	}

	struct http_response *response =
	    http_get("https://news.ycombinator.com/rss", NULL, 5);

	if (response == NULL) {
		fputs("error: http_get\n", stderr);
		return 1;
	}

	char *xml_part = strchr(response->body, '<');
	if (xml_part == NULL) {
		fputs("error: did not get XML response\n", stderr);
		http_response_free(response);
		return 1;
	}
	char *xml_end = strrchr(xml_part, '>');
	if (xml_end == NULL) {
		fputs("error: did not get XML response\n", stderr);
		http_response_free(response);
		return 1;
	}
	size_t xml_len = (xml_end - xml_part) + 1;
	xml_end[1] = '\0';

	xmlDoc *doc = xmlReadMemory(xml_part, xml_len, NULL, NULL, 0);

	http_response_free(response);

	if (doc == NULL) {
		fputs("error: failed to parse XML response\n", stderr);
		return 1;
	}

	xmlNode *root = xmlDocGetRootElement(doc);
	if (root == NULL || root->children == NULL) {
		fputs("error: unexpected XML structure\n", stderr);
		xmlFreeDoc(doc);
		return 1;
	}
	xmlNode *channel = root->children;

	FILE *urlfile = fopen(path, "a");
	if (urlfile == NULL) {
		fprintf(stderr, "error: fopen: %s\n", path);
		xmlFreeDoc(doc);
		return 1;
	}

	int index = 0;
	for (xmlNode *node = channel->children; node; node = node->next) {
		grab_item(urlfile, termsize.ws_col, doc, node, &index);
		if (index == (termsize.ws_row - 1))
			break; // Print only enough to fill the screen.
	}

	fclose(urlfile);
	xmlFreeDoc(doc);

	return 0;
}

static inline int is_wsl(void) {
	return getenv("WSL_DISTRO_NAME") != NULL;
}

void open_item(const char *path, unsigned long target) {
	char url[MAXURL];

	FILE *fd = fopen(path, "r");
	if (fd == NULL) {
		fprintf(stderr, "error: fopen: %s\n", path);
		return;
	}
	for (unsigned long i = 1; i <= target; i += 1) {
		if (fgets(url, MAXURL, fd) == NULL) {
			fprintf(stderr, "invalid index: %lu\n", target);
			fclose(fd);
			return;
		}
	}
	fclose(fd);
	url[strcspn(url, "\n")] = '\0';

	pid_t pid = fork();
	if (pid == -1) {
		fputs("error: fork\n", stderr);
		return;
	}
	if (pid == 0) {
#if defined(__APPLE__)
		execlp("open", "open", url, NULL);
#elif defined(_WIN32)
		execlp("powershell.exe", "powershell.exe", "-Command", "Start-Process", url, NULL);
#elif defined(__linux__)
		if (is_wsl())
			execlp("powershell.exe", "powershell.exe", "-Command", "Start-Process", url, NULL);
		else
			execlp("xdg-open", "xdg-open", url, NULL);
#else
		fputs("OS not supported\n", stderr);
		_exit(1);
#endif
		_exit(1);
	}
	waitpid(pid, NULL, 0);
}

int main(int argc, char **argv) {
	const char *home = getenv("HOME");
	if (home == NULL) {
		fputs("error: HOME is not set\n", stderr);
		return 1;
	}
	char *path;
	if (asprintf(&path, "%s/.hn", home) == -1) {
		fputs("error: asprintf\n", stderr);
		return 1;
	}

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
	return 0;
}
