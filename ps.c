/* See LICENSE file for copyright and license details. */
#include <sys/sysinfo.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <pwd.h>
#include "proc.h"
#include "util.h"

static void usage(void);
static void psout(struct procstat *ps);
static void psr(const char *path);

enum {
	PS_aflag = 1 << 0,
	PS_Aflag = 1 << 1,
	PS_dflag = 1 << 2,
	PS_fflag = 1 << 3
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
		flags |= PS_fflag;
		break;
	default:
		usage();
	} ARGEND;

	if (!(flags & PS_fflag))
		printf("  PID TTY          TIME CMD\n");
	else
		printf("UID        PID  PPID  C STIME TTY          TIME CMD\n");
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
	char cmdline[BUFSIZ], *cmd;
	char stimestr[6];
	char *ttystr, *myttystr;
	int tty_maj, tty_min;
	uid_t myeuid, peuid, puid;
	unsigned sut;
	struct sysinfo info;
	struct tm *tm;
	time_t start;
	struct passwd *pw;

	/* Ignore session leaders */
	if (flags & PS_dflag)
		if (ps->pid == ps->sid)
			return;

	sut = (ps->stime + ps->utime) / 100;

	devtotty(ps->tty_nr, &tty_maj, &tty_min);
	ttystr = ttytostr(tty_maj, tty_min);
	/* Only print processes that are associated with
	 * a terminal and they are not session leaders */
	if (flags & PS_aflag) {
		if (ps->pid == ps->sid || ttystr[0] == '?') {
			free(ttystr);
			return;
		}
	}

	if (!(flags & (PS_aflag | PS_Aflag | PS_dflag))) {
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

	procuid(ps->pid, &puid);
	errno = 0;
	pw = getpwuid(puid);
	if (errno || !pw)
		eprintf("getpwuid %d:", puid);

	sysinfo(&info);
	start = time(NULL) - (info.uptime - (ps->starttime / 100));
	tm = localtime(&start);
	strftime(stimestr, sizeof(stimestr),
		 "%H:%M", tm);
	if (parsecmdline(ps->pid, cmdline, sizeof(cmdline)) < 0)
		cmd = ps->comm;
	else
		cmd = cmdline;

	if (!(flags & PS_fflag))
		printf("%5d %-6s   %02u:%02u:%02u %s\n", ps->pid, ttystr,
		       sut / 3600, (sut % 3600) / 60, sut % 60, ps->comm);
	else {
		printf("%-8s %5d %5d  ? %5s %-5s    %02u:%02u:%02u %s%s%s\n",
		       pw->pw_name, ps->pid,
		       ps->ppid, stimestr, ttystr,
		       sut / 3600, (sut % 3600) / 60, sut % 60,
		       (cmd == ps->comm) ? "[" : "", cmd,
		       (cmd == ps->comm) ? "]" : "");
	}
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
