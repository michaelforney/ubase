/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: respawn [-d N] cmd [args...]\n");
}

int
main(int argc, char *argv[])
{
	pid_t pid;
	int savederrno;
	unsigned int delay = 0;

	ARGBEGIN {
	case 'd':
		delay = estrtol(EARGF(usage()), 0);
		break;
	default:
		usage();
	} ARGEND;

	if(argc < 1)
		usage();

	while (1) {
		pid = fork();
		if (pid < 0)
			eprintf("fork:");
		switch (pid) {
		case 0:
			if (setsid() < 0)
				eprintf("setsid:");
			execvp(argv[0], argv);
			savederrno = errno;
			weprintf("execvp %s:", argv[0]);
			_exit(savederrno == ENOENT ? 127 : 126);
			break;
		default:
			waitpid(pid, NULL, 0);
			break;
		}
		sleep(delay);
	}
	/* not reachable */
	return EXIT_SUCCESS;
}
