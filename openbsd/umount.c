#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include "../ubase.h"
#include "../util.h"

int
do_umount(const char *target, int opts)
{
	int flags = 0;

	if (opts & UBASE_MNT_FORCE)
		flags |= MNT_FORCE;
	return unmount(target, flags);
}
