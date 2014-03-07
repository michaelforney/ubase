/* See LICENSE file for copyright and license details. */
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-alfn] target\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	int aflag = 0;
	int flags = 0;
	int ret = EXIT_SUCCESS;
	FILE *fp;
	struct mntent *me;

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
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

	if (argc < 1 && aflag == 0)
		usage();

	if (aflag == 1) {
		fp = setmntent("/etc/fstab", "r");
		if (!fp)
			eprintf("setmntent %s:", "/etc/fstab");
		while ((me = getmntent(fp))) {
			if (umount2(me->mnt_dir, flags) < 0) {
				perror("umount2:");
				ret = EXIT_FAILURE;
			}
		}
		endmntent(fp);
		return ret;
	}

	for (i = 0; i < argc; i++) {
		if (umount2(argv[i], flags) < 0) {
			perror("umount2:");
			ret = EXIT_FAILURE;
		}
	}
	return ret;
}
