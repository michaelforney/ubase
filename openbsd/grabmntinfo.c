/* See LICENSE file for copyright and license details. */
#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ubase.h"
#include "../util.h"

int
grabmntinfo(struct mntinfo **minfo)
{
	int siz, i;
	struct statfs *mntbuf;
	struct mntinfo *mi;

	siz = getmntinfo(&mntbuf, MNT_WAIT);
	if (!siz)
		eprintf("getmntinfo:");
	mi = malloc(siz * sizeof(*mi));
	if (!mi)
		eprintf("malloc:");
	for (i = 0; i < siz; i++) {
		mi[i].fsname = strdup(mntbuf[i].f_mntfromname);
		mi[i].mntdir = strdup(mntbuf[i].f_mntonname);
	}
	*minfo = mi;
	return siz;
}
