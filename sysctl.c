/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

static int
getsysctl(char *variable, char **value)
{
	char path[PATH_MAX];
	char *p;
	char *buf, *tmp, c;
	int fd;
	ssize_t n;
	size_t sz, i;

	for (p = variable; *p; p++)
		if (*p == '.')
			*p = '/';

	strlcpy(path, "/proc/sys/", sizeof(path));
	if (strlcat(path, variable, sizeof(path)) >= sizeof(path))
		return -1;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;

	i = 0;
	sz = 1;
	buf = NULL;
	while (1) {
		n = read(fd, &c, 1);
		if (n < 0) {
			close(fd);
			free(buf);
			return -1;
		}
		if (n == 0)
			break;
		if (i == sz - 1) {
			sz *= 2;
			tmp = realloc(buf, sz);
			if (!tmp) {
				close(fd);
				free(buf);
				return -1;
			}
			buf = tmp;
		}
		buf[i++] = c;
	}
	buf[i] = '\0';

	p = strrchr(buf, '\n');
	if (p)
		*p = '\0';

	*value = buf;

	close(fd);

	return 0;
}

static int
setsysctl(char *variable, char *value)
{
	char path[PATH_MAX];
	char *p;
	int fd;
	ssize_t n;

	for (p = variable; *p; p++)
		if (*p == '.')
			*p = '/';

	strlcpy(path, "/proc/sys/", sizeof(path));
	if (strlcat(path, variable, sizeof(path)) >= sizeof(path))
		return -1;

	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;

	n = write(fd, value, strlen(value));
	if (n != (ssize_t)strlen(value)) {
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static void
usage(void)
{
	eprintf("usage: %s variable[=value]...\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *variable;
	char *value;
	char *p;
	int i;

	argv0 = argv[0];
	argv++;
	argc--;

	if (argc < 1)
		usage();

	for (i = 0; i < argc; i++) {
		value = NULL;
		variable = argv[i];
		p = strchr(variable, '=');
		if (p) {
			if (p[1] == '\0') {
				weprintf("malformed sysctl: %s\n", argv[i]);
				continue;
			}
			*p = '\0';
			value = &p[1];
		}
		if (variable) {
			if (value) {
				if (setsysctl(variable, value) < 0) {
					weprintf("failed to set sysctl for %s\n", variable);
					continue;
				}
			}
			else {
				if (getsysctl(variable, &value) < 0) {
					weprintf("failed to get sysctl for %s\n", variable);
					continue;
				}
				printf("%s = %s\n", variable, value);
				free(value);
			}
		}
	}

	return EXIT_SUCCESS;
}
