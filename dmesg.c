/* See LICENSE file for copyright and license details. */
#include <sys/klog.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static int dmesg_show(int fd, const void *buf, size_t n);

enum {
	SYSLOG_ACTION_READ_ALL = 3,
	SYSLOG_ACTION_CLEAR = 5,
	SYSLOG_ACTION_CONSOLE_LEVEL = 8,
	SYSLOG_ACTION_SIZE_BUFFER = 10
};

static void
usage(void)
{
	eprintf("usage: [-Ccr] [-n level] %s\n", argv0);
}

int
main(int argc, char *argv[])
{
	int n;
	char *buf;
	int cflag = 0;
	long level;

	ARGBEGIN {
	case 'C':
		if (klogctl(SYSLOG_ACTION_CLEAR, NULL, 0) < 0)
			eprintf("klogctl:");
		return EXIT_SUCCESS;
	case 'c':
		cflag = 1;
		break;
	case 'r':
		break;
	case 'n':
		level = estrtol(EARGF(usage()), 10);
		if (klogctl(SYSLOG_ACTION_CONSOLE_LEVEL, NULL, level) < 0)
			eprintf("klogctl:");
		return EXIT_SUCCESS;
	default:
		usage();
	} ARGEND;

	n = klogctl(SYSLOG_ACTION_SIZE_BUFFER, NULL, 0);
	if (n < 0)
		eprintf("klogctl:");

	buf = emalloc(n);

	n = klogctl(SYSLOG_ACTION_READ_ALL, buf, n);
	if (n < 0)
		eprintf("klogctl:");

	n = dmesg_show(STDOUT_FILENO, buf, n);
	if (n < 0)
		eprintf("dmesg_show:");

	if (cflag && klogctl(SYSLOG_ACTION_CLEAR, NULL, 0) < 0)
		eprintf("klogctl:");

	free(buf);
	return EXIT_SUCCESS;
}

static int
dmesg_show(int fd, const void *buf, size_t n)
{
	const char *p = buf;
	ssize_t r;

	r = write(fd, p, n);
	if (r < 0 || (size_t)r != n)
		return -1;
	if (p[n - 1] != '\n')
		putchar('\n');
	return 0;
}
