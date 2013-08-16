/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grabmntinfo.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s target\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	struct mntinfo *minfo = NULL;
	int siz;
	int ret = 0;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	siz = grabmntinfo(&minfo);
	if (!siz)
		eprintf("grabmntinfo:");
	for (i = 0; i < siz; i++)
		if (!strcmp(minfo[i].mntdir, argv[0]))
			break;
	free(minfo);

	if (i == siz)
		ret = 1;

	return ret;
}
