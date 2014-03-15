/* See LICENSE file for copyright and license details. */
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include "util.h"

static int umountall(int);

static void
usage(void)
{
	weprintf("usage: %s [-lfn] target\n", argv0);
	weprintf("usage: %s -a [-lfn]\n", argv0);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int i;
	int aflag = 0;
	int flags = 0;
	int ret = EXIT_SUCCESS;

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

	if (aflag == 1)
		return umountall(flags);

	for (i = 0; i < argc; i++) {
		if (umount2(argv[i], flags) < 0) {
			weprintf("umount2 %s:", argv[i]);
			ret = EXIT_FAILURE;
		}
	}
	return ret;
}

static int
umountall(int flags)
{
	FILE *fp;
	struct mntent *me;
	int ret;
	char **mntdirs = NULL;
	int len = 0;

	fp = setmntent("/etc/mtab", "r");
	if (!fp)
		eprintf("setmntent %s:", "/etc/mtab");
	while ((me = getmntent(fp))) {
		if (strcmp(me->mnt_type, "proc") == 0)
			continue;
		mntdirs = realloc(mntdirs, ++len * sizeof(*mntdirs));
		if (!mntdirs)
			eprintf("realloc:");
		mntdirs[len - 1] = strdup(me->mnt_dir);
		if (!mntdirs[len - 1])
			eprintf("strdup:");
	}
	endmntent(fp);
	while (--len >= 0) {
		if (umount2(mntdirs[len], flags) < 0) {
			weprintf("umount2 %s:", mntdirs[len]);
			ret = EXIT_FAILURE;
		}
		free(mntdirs[len]);
	}
	free(mntdirs);
	return ret;
}
