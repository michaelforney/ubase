/* See LICENSE file for copyright and license details. */
#include <sys/ioctl.h>
#include <sys/types.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

#define KDGKBTYPE	0x4B33	/* get keyboard type */

#define VT_ACTIVATE	0x5606	/* make vt active */
#define VT_WAITACTIVE	0x5607	/* wait for vt active */

static char *vts[] = {
	"/proc/self/fd/0",
	"/dev/console",
	"/dev/tty",
	"/dev/tty0",
};

static void
usage(void)
{
	eprintf("usage: chvt N\n");
}

int
main(int argc, char *argv[])
{
	unsigned int n, i;
	int fd;
	char c;

	if (argc != 2 || strspn(argv[1], "1234567890") != strlen(argv[1]))
		usage();

	n = estrtol(argv[1], 10);
	for (i = 0; i < LEN(vts); i++) {
		fd = open(vts[i], O_RDONLY);
		if (fd < 0)
			continue;
		c = 0;
		if (ioctl(fd, KDGKBTYPE, &c) == 0)
			goto VTfound;
		close(fd);
	}

	eprintf("couldn't find a console.\n");
VTfound:
	if (ioctl(fd, VT_ACTIVATE, n) == -1)
		eprintf("VT_ACTIVATE %d:", n);
	if (ioctl(fd, VT_WAITACTIVE, n) == -1)
		eprintf("VT_WAITACTIVE %d:", n);
	close(fd);

	return EXIT_SUCCESS;
}
