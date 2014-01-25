/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grabmntinfo.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-dqx] target\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	int qflag = 0, dflag = 0, xflag = 0;
	struct mntinfo *minfo = NULL;
	int siz;
	int ret = 0;
	struct stat st1, st2;

	ARGBEGIN {
	case 'q':
		qflag = 1;
		break;
	case 'd':
		dflag = 1;
		break;
	case 'x':
		xflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	if (stat(argv[0], &st1) < 0)
		eprintf("stat %s:", argv[0]);

	if (xflag) {
		if (!S_ISBLK(st1.st_mode))
			eprintf("stat: %s: not a block device\n",
				argv[0]);
		printf("%u:%u\n", major(st1.st_rdev),
		       minor(st1.st_rdev));
		return EXIT_SUCCESS;
	}

	if (!S_ISDIR(st1.st_mode))
		eprintf("stat %s: not a directory\n", argv[0]);

	if (dflag) {
		printf("%u:%u\n", major(st1.st_dev),
		       minor(st1.st_dev));
		return EXIT_SUCCESS;
	}

	siz = grabmntinfo(&minfo);
	if (!siz)
		eprintf("grabmntinfo:");
	for (i = 0; i < siz; i++) {
		if (stat(minfo[i].mntdir, &st2) < 0)
			eprintf("stat %s:", minfo[i].mntdir);
		if (st1.st_dev == st2.st_dev &&
		    st1.st_ino == st2.st_ino)
			break;
	}

	for (i = 0; i < siz; i++) {
		free(minfo[i].fsname);
		free(minfo[i].mntdir);
	}
	free(minfo);

	if (i == siz)
		ret = 1;

	if (!qflag)
		printf("%s %s a mountpoint\n", argv[0],
		       !ret ? "is" : "is not");

	return ret;
}
