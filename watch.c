/* See LICENSE file for copyright and license details. */
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-t] [-n interval] command\n", argv0);
}

int
main(int argc, char *argv[])
{
	char cmd[BUFSIZ];
	int i, interval = 2;

	ARGBEGIN {
	case 't':
		break;
	case 'n':
		/* Only whole seconds for now */
		interval = estrtol(EARGF(usage()), 10);
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	strlcpy(cmd, argv[0], sizeof(cmd));
	for (i = 1; i < argc; i++) {
		strlcat(cmd, " ", sizeof(cmd));
		strlcat(cmd, argv[i], sizeof(cmd));
	}

	for (;;) {
		printf("\e[2J\e[H");
		fflush(NULL);
		system(cmd);
		sleep(interval);
	}
	return EXIT_SUCCESS;
}
