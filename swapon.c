/* See LICENSE file for copyright and license details. */
#include <sys/swap.h>

#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-dp] [-a] device\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	int ret = EXIT_SUCCESS;
	int flags = 0;
	int all = 0;

	ARGBEGIN {
	case 'a':
		all = 1;
		break;
	case 'd':
		flags |= SWAP_FLAG_DISCARD;
		break;
	case 'p':
		flags |= SWAP_FLAG_PREFER;
		break;
	default:
		usage();
	} ARGEND;

	if (!all && argc < 1)
		usage();

	if (all) {
		struct mntent *me = NULL;
		FILE *fp;

		fp = setmntent("/etc/fstab", "r");
		if (!fp)
			eprintf("setmntent %s:", "/etc/fstab");
		while ((me = getmntent(fp)) != NULL) {
			if (strcmp(me->mnt_type, MNTTYPE_SWAP) == 0
			    && (hasmntopt(me, MNTOPT_NOAUTO) == NULL)) {
				if (swapon(me->mnt_fsname, flags) < 0) {
					weprintf("swapon %s:", me->mnt_fsname);
					ret = EXIT_FAILURE;
				}
			}
		}
		endmntent(fp);
	} else {
		for (i = 0; i < argc; i++) {
			if (swapon(argv[i], flags) < 0) {
				weprintf("swapon %s:", argv[i]);
				ret = EXIT_FAILURE;
			}
		}
	}
	return ret;
}
