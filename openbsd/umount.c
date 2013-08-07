/* See LICENSE file for copyright and license details. */
#include <sys/param.h>
#include <sys/mount.h>
#include <errno.h>
#include <stdio.h>
#include "../ubase.h"
#include "../util.h"

int
do_umount(const char *target, int opts)
{
	int flags = 0;

	if (opts & UBASE_MNT_FORCE)
		flags |= MNT_FORCE;
	if (opts & UBASE_MNT_DETACH) {
		errno = ENOTSUP;
		return -1;
	}
	return unmount(target, flags);
}
