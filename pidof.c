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
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-o pid1,pid2,...pidN] [-s] [program...]\n", argv0);
}

static struct omit {
	pid_t pid;
	struct omit *next;
} *omithead;

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
	struct omit *onode, *tmp;

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

	for (p = strtok(arg, ","); p; p = strtok(NULL, ",")) {
		onode = emalloc(sizeof(*onode));
		if (strcmp(p, "%PPID") == 0)
			onode->pid = getppid();
		else
			onode->pid = estrtol(p, 10);
		onode->next = omithead;
		omithead = onode;
	}

	if (!(dp = opendir("/proc")))
		eprintf("opendir /proc:");

	while ((entry = readdir(dp))) {
		if (!pidfile(entry->d_name))
			continue;
		pid = estrtol(entry->d_name, 10);
		if (oflag) {
			for (onode = omithead; onode; onode = onode->next)
				if (onode->pid == pid)
					break;
			if (onode)
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

	onode = omithead;
	while (onode) {
		tmp = onode->next;
		free(onode);
		onode = tmp;
	}

	return EXIT_SUCCESS;
}
