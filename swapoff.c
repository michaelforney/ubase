/* See LICENSE file for copyright and license details. */
#include <sys/swap.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s device\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	int ret = 0;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (i = 0; i < argc; i++) {
		ret = swapoff(argv[i]);
		if (ret < 0) {
			fprintf(stderr, "swapoff %s: %s\n",
				argv[i], strerror(errno));
			ret = 1;
		}
	}
	return ret;
}
