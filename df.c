/* See LICENSE file for copyright and license details. */
#include <sys/statvfs.h>
#include <stdio.h>
#include <stdlib.h>
#include "grabmntinfo.h"
#include "util.h"

static void mnt_show(const char *fsname, const char *dir);

static void
usage(void)
{
	eprintf("usage: %s\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct mntinfo *minfo = NULL;
	int siz, i;

	ARGBEGIN {
	case 'a':
		break;
	case 's':
	case 'h':
	case 'i':
		eprintf("not implemented\n");
	default:
		usage();
	} ARGEND;

	printf("Filesystem  512-blocks      Used     Avail Capacity  Mounted on\n");
	siz = grabmntinfo(&minfo);
	if (!siz)
		eprintf("grabmntinfo:");

	for (i = 0; i < siz; i++)
		mnt_show(minfo[i].fsname, minfo[i].mntdir);
	free(minfo);

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
