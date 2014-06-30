/* See LICENSE file for copyright and license details. */
#include <sys/syscall.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "reboot.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-hs]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int hflag = 0;
	int sflag = 0;
	int cmd;

	ARGBEGIN {
	case 'h':
		hflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 0 || (hflag ^ sflag) == 0)
		usage();

	cmd = hflag ? LINUX_REBOOT_CMD_CAD_ON : LINUX_REBOOT_CMD_CAD_OFF;

	if (syscall(__NR_reboot, LINUX_REBOOT_MAGIC1,
		    LINUX_REBOOT_MAGIC2, cmd, NULL) < 0)
		eprintf("reboot:");
	return EXIT_SUCCESS;
}
