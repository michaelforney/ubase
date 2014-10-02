/* See LICENSE file for copyright and license details. */
#include <sys/statvfs.h>

#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void mnt_show(const char *fsname, const char *dir);

static void
usage(void)
{
	eprintf("usage: %s [-a]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct mntent *me = NULL;
	FILE *fp;
	int aflag = 0;

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	case 's':
	case 'h':
	case 'i':
		eprintf("not implemented\n");
	default:
		usage();
	} ARGEND;

	printf("Filesystem  512-blocks      Used     Avail Capacity  Mounted on\n");

	fp = setmntent("/proc/mounts", "r");
	if (!fp)
		eprintf("setmntent %s:", "/proc/mounts");
	while ((me = getmntent(fp)) != NULL) {
		if (aflag == 0)
			if (strcmp(me->mnt_type, "rootfs") == 0)
				continue;
		mnt_show(me->mnt_fsname, me->mnt_dir);
	}
	endmntent(fp);

	return 0;
}

static void
mnt_show(const char *fsname, const char *dir)
{
	struct statvfs s;
	unsigned long long total, used, avail;
	int capacity = 0;
	int bs;

	statvfs(dir, &s);

	bs = s.f_frsize / 512;
	total = s.f_blocks * bs;
	avail = s.f_bfree * bs;
	used = total - avail;

	if (used + avail) {
		capacity = (used * 100) / (used + avail);
		if (used * 100 != capacity * (used + avail))
			capacity++;
	}

	printf("%-12s %9llu %9llu %9llu %7d%%  %s\n",
	       fsname, total, used, avail, capacity,
	       dir);
}
