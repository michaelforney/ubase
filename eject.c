/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "util.h"

enum {
	CDROM_EJECT = 0x5309,
	CDROM_CLOSE_TRAY = 0x5319,
};

static void
usage(void)
{
	eprintf("usage: %s [-t]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int fd, out;
	const char *cdrom = "/dev/sr0";
	int tflag = 0;

	ARGBEGIN {
	case 't':
		tflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	fd = open(cdrom, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
		eprintf("open %s:", cdrom);
	if (tflag) {
		if (ioctl(fd, CDROM_CLOSE_TRAY, &out) < 0)
			eprintf("ioctl:");
	} else {
		if (ioctl(fd, CDROM_EJECT, &out) < 0)
			eprintf("ioctl:");
	}
	close(fd);
	return 0;
}
