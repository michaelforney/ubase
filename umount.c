/* See LICENSE file for copyright and license details. */
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-lfn] target\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	int flags = 0;
	int ret = EXIT_SUCCESS;

	ARGBEGIN {
	case 'f':
		flags |= MNT_FORCE;
		break;
	case 'l':
		flags |= MNT_DETACH;
		break;
	case 'n':
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (i = 0; i < argc; i++) {
		if (umount2(argv[i], flags) < 0)
			perror("umount2:");
		ret = EXIT_FAILURE;
	}
	return ret;
}
