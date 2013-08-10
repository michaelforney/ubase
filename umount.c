/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include "ubase.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-lf] target\n", argv0);
}

int
main(int argc, char *argv[]) {
	int i;
	int flags = 0;
	int ret = 0;

	ARGBEGIN {
	case 'f':
		flags |= UBASE_MNT_FORCE;
		break;
	case 'l':
		flags |= UBASE_MNT_DETACH;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (i = 0; i < argc; i++) {
		if (do_umount(argv[i], flags) < 0)
			eprintf("do_umount:");
		ret = 1;
	}
	return ret;
}
