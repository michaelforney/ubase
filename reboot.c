/* See LICENSE file for copyright and license details. */
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include "reboot.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s\n", argv0);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc > 0)
		usage();

	sync();

	if (syscall(__NR_reboot, 0xfee1dead, 672274793,
		    LINUX_REBOOT_CMD_RESTART, NULL) < 0)
		eprintf("reboot:");
	return 0;
}
