/* See LICENSE file for copyright and license details. */
#include <sys/klog.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	eprintf("usage: [-Ccnr] %s\n", argv0);
}

int
main(int argc, char *argv[])
{
	int n;
	char *buf;
	int cflag = 0;
	int rflag = 0;
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
		rflag = 1;
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

	buf = malloc(n);
	if (!buf)
		eprintf("malloc:");

	n = klogctl(SYSLOG_ACTION_READ_ALL, buf, n);
	if (n < 0)
		eprintf("klogctl:");

	if (rflag) {
		if (write(STDOUT_FILENO, buf, n) != n)
			eprintf("write:");
	} else {
		n = dmesg_show(STDOUT_FILENO, buf, n);
		if (n < 0)
			eprintf("dmesg_show:");
	}

	if (cflag && klogctl(SYSLOG_ACTION_CLEAR, NULL, 0) < 0)
		eprintf("klogctl:");

	free(buf);
	return EXIT_SUCCESS;
}

static int
dmesg_show(int fd, const void *buf, size_t n)
{
	int last = '\n';
	char newbuf[n], *q = newbuf;
	const char *p = buf;
	size_t i;

	memset(newbuf, 0, n);
	for (i = 0; i < n; ) {
		if (last == '\n' && p[i] == '<') {
			i += 2;
			if (i + 1 < n && p[i + 1] == '>')
				i++;
		} else {
			*q++ = p[i];
		}
		last = p[i++];
	}
	if (write(fd, newbuf, n) != n)
		return -1;
	if (last != '\n')
		if (write(fd, "\n", 1) != 1)
			return -1;
	return 0;
}
