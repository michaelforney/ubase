/* See LICENSE file for copyright and license details. */
#include <sys/swap.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-d] device\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	int ret = 0;
	int flags = 0;

	ARGBEGIN {
	case 'd':
		flags |= SWAP_FLAG_DISCARD;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (i = 0; i < argc; i++) {
		ret = swapon(argv[i], flags);
		if (ret < 0) {
			fprintf(stderr, "swapon %s: %s\n",
				argv[i], strerror(errno));
			ret = 1;
		}
	}
	return ret;
}
