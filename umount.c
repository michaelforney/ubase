/* See LICENSE file for copyright and license details. */
#include <sys/mount.h>
#include <stdio.h>
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
	int ret = 0;

	ARGBEGIN {
	case 'f':
		flags |= MNT_FORCE;
		break;
	case 'l':
		flags |= MNT_DETACH;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (i = 0; i < argc; i++) {
		if (umount2(argv[i], flags) < 0)
			eprintf("umount2:");
		ret = 1;
	}
	return ret;
}
