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

	n = dmesg_size();
	if (n < 0)
		eprintf("dmesg_size:");
	buf = malloc(n);
	if (!buf)
		eprintf("malloc:");
	n = dmesg_read(buf, n);
	if (n < 0)
		eprintf("dmesg_read:");
	n = dmesg_show(STDOUT_FILENO, buf, n);
	if (n < 0)
		eprintf("dmesg_show:");
	free(buf);
	return 0;
}
