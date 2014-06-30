/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-o offset] -l length file\n", argv0);
}

int
main(int argc, char *argv[])
{
	int fd;
	off_t size = 0, offset = 0;

	ARGBEGIN {
	case 'l':
		size = estrtol(EARGF(usage()), 10);
		break;
	case 'o':
		offset = estrtol(EARGF(usage()), 10);
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1 || !size)
		usage();

	fd = open(argv[0], O_RDWR | O_CREAT, 0644);
	if (fd < 0)
		eprintf("open %s:", argv[0]);

	if (posix_fallocate(fd, offset, size) < 0)
		eprintf("posix_fallocate:");

	close(fd);
	return EXIT_SUCCESS;
}
