/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include "proc.h"
#include "util.h"

struct {
	const char *name;
	int sig;
} sigs[] = {
#define SIG(n) { #n, SIG##n }
	SIG(ABRT), SIG(ALRM), SIG(BUS),  SIG(CHLD), SIG(CONT), SIG(FPE),  SIG(HUP),
	SIG(ILL),  SIG(INT),  SIG(KILL), SIG(PIPE), SIG(QUIT), SIG(SEGV), SIG(STOP),
	SIG(TERM), SIG(TSTP), SIG(TTIN), SIG(TTOU), SIG(USR1), SIG(USR2), SIG(URG),
#undef SIG
};

static void
usage(void)
{
	eprintf("usage: %s [-s signal]\n", argv0);
}

int
main(int argc, char *argv[])
{
	DIR *dp;
	struct dirent *entry;
	char *end, *v;
	int sig = SIGTERM;
	pid_t pid;
	size_t i;

	ARGBEGIN {
	case 's':
		v = EARGF(usage());
		sig = strtol(v, &end, 0);
		if(*end == '\0')
			break;
		for(i = 0; i < LEN(sigs); i++) {
			if(strcasecmp(v, sigs[i].name) == 0) {
				sig = sigs[i].sig;
				break;
			}
		}
		if(i == LEN(sigs))
			eprintf("%s: unknown signal\n", v);
		break;
	default:
		usage();
	} ARGEND;

	if (sig != SIGSTOP && sig != SIGCONT)
		kill(-1, SIGSTOP);

	if (!(dp = opendir("/proc")))
		eprintf("opendir /proc:");
	while ((entry = readdir(dp))) {
		if (!pidfile(entry->d_name))
			continue;
		pid = estrtol(entry->d_name, 10);
		if (pid == 1 || pid == getpid() ||
		    getsid(pid) == getsid(0) || getsid(pid) == 0)
			continue;
		kill(pid, sig);
	}
	closedir(dp);

	if (sig != SIGSTOP && sig != SIGCONT)
		kill(-1, SIGCONT);

	return EXIT_SUCCESS;
}
