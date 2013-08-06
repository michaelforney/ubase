/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include "../ubase.h"
#include "../util.h"

int
grabmntinfo(struct mntinfo **minfo)
{
	struct mntent *me;
	struct mntinfo *mi = NULL;
	int siz = 0;
	FILE *fp;

	fp = setmntent("/proc/mounts", "r");
	if (!fp)
		eprintf("setmntent:");
	while ((me = getmntent(fp))) {
		mi = realloc(mi, (siz + 1) * sizeof(*mi));
		if (!mi)
			eprintf("realloc:");
		mi[siz].fsname = strdup(me->mnt_fsname);
		mi[siz].mntdir = strdup(me->mnt_dir);
		siz++;
	}
	endmntent(fp);
	*minfo = mi;
	return siz;
}
