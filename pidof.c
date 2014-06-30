/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "proc.h"
#include "queue.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-o pid1,pid2,...pidN] [-s] [program...]\n", argv0);
}

struct pidentry {
	pid_t pid;
	TAILQ_ENTRY(pidentry) entry;
};

static TAILQ_HEAD(omitpid_head, pidentry) omitpid_head;

int
main(int argc, char *argv[])
{
	DIR *dp;
	struct dirent *entry;
	pid_t pid;
	struct procstat ps;
	char cmdline[BUFSIZ], *cmd, *p, *arg = NULL;
	int i, found = 0;
	int sflag = 0, oflag = 0;
	struct pidentry *pe, *tmp;

	ARGBEGIN {
	case 's':
		sflag = 1;
		break;
	case 'o':
		oflag = 1;
		arg = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		return 1;

	TAILQ_INIT(&omitpid_head);

	for (p = strtok(arg, ","); p; p = strtok(NULL, ",")) {
		pe = emalloc(sizeof(*pe));
		if (strcmp(p, "%PPID") == 0)
			pe->pid = getppid();
		else
			pe->pid = estrtol(p, 10);
		TAILQ_INSERT_TAIL(&omitpid_head, pe, entry);
	}

	if (!(dp = opendir("/proc")))
		eprintf("opendir /proc:");

	while ((entry = readdir(dp))) {
		if (!pidfile(entry->d_name))
			continue;
		pid = estrtol(entry->d_name, 10);
		if (oflag) {
			TAILQ_FOREACH(pe, &omitpid_head, entry)
				if (pe->pid == pid)
					break;
			if (pe)
				continue;
		}
		if (parsestat(pid, &ps) < 0)
			continue;
		if (parsecmdline(ps.pid, cmdline,
				 sizeof(cmdline)) < 0) {
			cmd = ps.comm;
		} else {
			if ((p = strchr(cmdline, ' ')))
				*p = '\0';
			cmd = basename(cmdline);
		}
		/* Workaround for login shells */
		if (cmd[0] == '-')
			cmd++;
		for (i = 0; i < argc; i++) {
			if (strcmp(cmd, argv[i]) == 0) {
				putword(entry->d_name);
				found++;
				if (sflag)
					goto out;
			}
		}
	}

out:
	if (found)
		putchar('\n');

	closedir(dp);

	for (pe = TAILQ_FIRST(&omitpid_head); pe; pe = tmp) {
		tmp = TAILQ_NEXT(pe, entry);
		free(pe);
	}

	return EXIT_SUCCESS;
}
