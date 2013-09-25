/* See LICENSE file for copyright and license details. */
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
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
static void psr(const char *file);

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
	struct procstatus pstatus;
	char cmdline[BUFSIZ], *cmd;
	char buf[BUFSIZ];
	char *ttystr, *myttystr;
	int tty_maj, tty_min;
	uid_t myeuid;
	unsigned sutime;
	time_t start;
	char stimestr[sizeof("%H:%M")];
	struct sysinfo info;
	struct passwd *pw;
	struct winsize w;

	/* Ignore session leaders */
	if (flags & PS_dflag)
		if (ps->pid == ps->sid)
			return;

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

	parsestatus(ps->pid, &pstatus);

	/* This is the default case, only print processes that have
	 * the same controlling terminal as the invoker and the same
	 * euid as the current user */
	if (!(flags & (PS_aflag | PS_Aflag | PS_dflag))) {
		myttystr = ttyname(STDIN_FILENO);
		if (myttystr) {
			if (strcmp(myttystr + strlen("/dev/"), ttystr)) {
				free(ttystr);
				return;
			}
		} else {
			/* The invoker has no controlling terminal - just
			 * go ahead and print the processes anyway */
			ttystr[0] = '?';
			ttystr[1] = '\0';
		}
		myeuid = geteuid();
		if (myeuid != pstatus.euid) {
			free(ttystr);
			return;
		}
	}

	sutime = (ps->stime + ps->utime) / sysconf(_SC_CLK_TCK);

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	if (!(flags & PS_fflag)) {
		snprintf(buf, sizeof(buf), "%5d %-6s   %02u:%02u:%02u %s", ps->pid, ttystr,
			 sutime / 3600, (sutime % 3600) / 60, sutime % 60,
			 ps->comm);
		if (w.ws_col)
			printf("%.*s\n", w.ws_col, buf);
		else
			printf("%s\n", buf);
	} else {
		errno = 0;
		pw = getpwuid(pstatus.uid);
		if (errno || !pw)
			eprintf("getpwuid %d:", pstatus.uid);

		if (sysinfo(&info) < 0)
			eprintf("sysinfo:");

		start = time(NULL) - info.uptime;
		start += (ps->starttime / sysconf(_SC_CLK_TCK));
		strftime(stimestr, sizeof(stimestr),
			 "%H:%M", localtime(&start));

		/* For kthreads/zombies /proc/<pid>/cmdline will be
		 * empty so use ps->comm in that case */
		if (parsecmdline(ps->pid, cmdline, sizeof(cmdline)) < 0)
			cmd = ps->comm;
		else
			cmd = cmdline;

		snprintf(buf, sizeof(buf), "%-8s %5d %5d  ? %5s %-5s    %02u:%02u:%02u %s%s%s",
			 pw->pw_name, ps->pid,
			 ps->ppid, stimestr, ttystr,
			 sutime / 3600, (sutime % 3600) / 60, sutime % 60,
			 (cmd == ps->comm) ? "[" : "", cmd,
			 (cmd == ps->comm) ? "]" : "");
		if (w.ws_col)
			printf("%.*s\n", w.ws_col, buf);
		else
			printf("%s\n", buf);
	}
	free(ttystr);
}

static void
psr(const char *file)
{
	struct procstat ps;
	pid_t pid;

	if (!pidfile(file))
		return;
	pid = estrtol(file, 10);
	parsestat(pid, &ps);
	psout(&ps);
}
