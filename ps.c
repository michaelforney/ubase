/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "proc.h"
#include "util.h"

static void usage(void);
static void psout(struct procstat *ps);
static void psr(const char *path);

enum {
	PS_aflag = 1 << 0,
	PS_Aflag = 1 << 1,
	PS_dflag = 1 << 2
};

static int flags;

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	case 'a':
		flags |= PS_aflag;
		break;
	case 'A':
		flags |= PS_Aflag;
		break;
	case 'd':
		flags |= PS_dflag;
		break;
	case 'e':
		flags |= PS_Aflag;
		break;
	case 'f':
		eprintf("not implemented\n");
	default:
		usage();
	} ARGEND;

	printf("  PID TTY          TIME CMD\n");
	recurse("/proc", psr);

	return 0;
}

static void
usage(void)
{
	eprintf("usage: [-aAdef] %s\n", argv0);
}

static void
psout(struct procstat *ps)
{
	char *ttystr, *myttystr;
	int tty_maj, tty_min;
	uid_t myeuid, peuid;
	unsigned sut;

	/* Ignore session leaders */
	if (flags & PS_dflag)
		if (ps->pid == ps->sid)
			return;

	sut = (ps->stime + ps->utime) / 100;

	devtotty(ps->tty_nr, &tty_maj, &tty_min);
	ttystr = ttytostr(tty_maj, tty_min);
	/* Only print processes that are associated with
	 * a terminal */
	if (ttystr[0] == '?' && (flags & PS_aflag)) {
		free(ttystr);
		return;
	}

	if (!flags) {
		myttystr = ttyname(STDIN_FILENO);
		if (myttystr) {
			if (strcmp(myttystr + strlen("/dev/"), ttystr)) {
				free(ttystr);
				return;
			}
		} else {
			ttystr[0] = '?';
			ttystr[1] = '\0';
		}
		proceuid(ps->pid, &peuid);
		myeuid = geteuid();
		if (myeuid != peuid) {
			free(ttystr);
			return;
		}
	}

	printf("%5d %-6s   %02u:%02u:%02u %s\n", ps->pid, ttystr,
	       sut / 3600, (sut % 3600) / 60, sut % 60, ps->comm);
	free(ttystr);
}

static void
psr(const char *path)
{
	struct procstat ps;
	pid_t pid;

	if (!validps(path))
		return;
	pid = estrtol(path, 10);
	parsestat(pid, &ps);
	psout(&ps);
}
