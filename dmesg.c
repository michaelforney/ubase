/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "ubase.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s\n", argv0);
}

int
main(int argc, char *argv[])
{
	int n;
	char *buf;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	n = syslog_size();
	if (n < 0)
		eprintf("syslog_size:");
	buf = malloc(n);
	if (!buf)
		eprintf("malloc:");
	n = syslog_read(buf, n);
	if (n < 0)
		eprintf("syslog_read:");
	if (write(STDOUT_FILENO, buf, n) != n)
		eprintf("write:");
	free(buf);
	return 0;
}
