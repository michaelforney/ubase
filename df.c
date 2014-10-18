/* See LICENSE file for copyright and license details. */
#include <sys/statvfs.h>

#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static long blksize = 512;
static int aflag = 0;
static int hflag = 0;
static int kflag = 0;

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

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	case 'h':
		hflag = 1;
		break;
	case 'k':
		kflag = 1;
		blksize = 1024;
		break;
	case 's':
	case 'i':
		eprintf("not implemented\n");
	default:
		usage();
	} ARGEND;

	if (kflag && hflag)
		hflag = 0;

	if (hflag)
		printf("Filesystem         Size       Used      "
		       "Avail Capacity   Mounted on\n");
	else
		printf("Filesystem  %ld-blocks      Used     "
		       "Avail Capacity  Mounted on\n", blksize);

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

#define CALC_POWER(n, power, base, i) do { \
	while (n > power) {                \
		power = power * base;      \
		i++;                       \
	}                                  \
} while(0)

static void
print_human(
	const char         *fsname,
	unsigned long long total,
	unsigned long long used,
	unsigned long long avail,
	int                capacity,
	const char         *dir)
{
	long base = 1024;
	unsigned long long power_total = base;
	unsigned long long power_used = base;
	unsigned long long power_avail = base;
	char postfixes[] = {'B', 'K', 'M', 'G', 'T', 'P', 'E'};
	int i = 0, j = 0, k = 0;

	total = total * blksize;
	used = used * blksize;
	avail = avail * blksize;

	CALC_POWER(total, power_total, base, i);
	CALC_POWER(used, power_used, base, j);
	CALC_POWER(avail, power_avail, base, k);

	total = i ? total / (power_total / base) : total;
	used = j ? used / (power_used / base) : used;
	avail = k ? avail / (power_avail / base) : avail;
	printf("%-12s %9llu%c %9llu%c %9llu%c %7d%%  %s\n",
	       fsname, total, postfixes[i], used, postfixes[j],
	       avail, postfixes[k], capacity, dir);
}

static void
mnt_show(const char *fsname, const char *dir)
{
	struct statvfs s;
	unsigned long long total, used, avail;
	int capacity = 0;
	int bs;

	statvfs(dir, &s);

	bs = s.f_frsize / blksize;
	total = s.f_blocks * bs;
	avail = s.f_bfree * bs;
	used = total - avail;

	if (used + avail) {
		capacity = (used * 100) / (used + avail);
		if (used * 100 != capacity * (used + avail))
			capacity++;
	}

	if (hflag)
		print_human(fsname, total, used, avail, capacity, dir);
	else
		printf("%-12s %9llu %9llu %9llu %7d%%  %s\n",
		       fsname, total, used, avail, capacity, dir);
}
