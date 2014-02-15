/* See LICENSE file for copyright and license details. */
#include <sys/swap.h>
#include <mntent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-a] device\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	int ret = EXIT_SUCCESS;
	int all = 0;

	ARGBEGIN {
	case 'a':
		all = 1;
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
		while ((me = getmntent(fp)) != NULL) {
			if (strcmp(me->mnt_type, MNTTYPE_SWAP) == 0) {
				if (swapoff(me->mnt_fsname) < 0) {
					fprintf(stderr, "swapoff %s: %s\n",
						me->mnt_fsname, strerror(errno));
					ret = EXIT_FAILURE;
				}
			}
		}
		endmntent(fp);
	} else {
		for (i = 0; i < argc; i++) {
			if (swapoff(argv[i]) < 0) {
				fprintf(stderr, "swapoff %s: %s\n",
					argv[i], strerror(errno));
				ret = EXIT_FAILURE;
			}
		}
	}
	return ret;
}
