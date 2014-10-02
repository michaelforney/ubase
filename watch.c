/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

	if (strlcpy(cmd, argv[0], sizeof(cmd)) >= sizeof(cmd))
		eprintf("command too long\n");
	for (i = 1; i < argc; i++) {
		if (strlcat(cmd, " ", sizeof(cmd)) >= sizeof(cmd))
			eprintf("command too long\n");
		if (strlcat(cmd, argv[i], sizeof(cmd)) >= sizeof(cmd))
			eprintf("command too long\n");
	}

	for (;;) {
		printf("\x1b[2J\x1b[H"); /* clear */
		fflush(NULL);
		system(cmd);
		sleep(interval);
	}
	return 0;
}
