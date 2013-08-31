/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include "proc.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-s] [program...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	DIR *dp;
	struct dirent *entry;
	pid_t pid;
	struct procstat ps;
	char cmdline[BUFSIZ], *cmd, *p;
	int i, found = 0;
	int sflag = 0;

	ARGBEGIN {
	case 's':
		sflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		return 1;

	if (!(dp = opendir("/proc")))
		eprintf("opendir /proc:");

	while ((entry = readdir(dp))) {
		if (!pidfile(entry->d_name))
			continue;
		for (i = 0; i < argc; i++) {
			pid = estrtol(entry->d_name, 10);
			parsestat(pid, &ps);
			if (parsecmdline(ps.pid, cmdline,
					 sizeof(cmdline)) < 0) {
				cmd = ps.comm;
			} else {
				if ((p = strchr(cmdline, ' ')))
					*p = '\0';
				cmd = basename(cmdline);
			}
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

	return 0;
}
