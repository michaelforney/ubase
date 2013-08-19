/* See LICENSE file for copyright and license details. */
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include "reboot.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-p]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int pflag = 0;
	int cmd = LINUX_REBOOT_CMD_HALT;

	ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 0)
		usage();

	sync();

	if (pflag)
		cmd = LINUX_REBOOT_CMD_POWER_OFF;

	if (syscall(__NR_reboot, 0xfee1dead, 672274793,
		    cmd, NULL) < 0)
		eprintf("reboot:");
	return 0;
}
