/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [tty] [term]\n", argv0);
}

static char *tty = "/dev/tty1";
static char *defaultterm = "TERM=linux";

int
main(int argc, char *argv[])
{
	int fd;
	struct sigaction sa;
	char term[128], logname[128], c;
	int i = 0;
	ssize_t n;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc > 2)
		usage();

	strlcpy(term, defaultterm, sizeof(term));
	if (argc > 0) {
		tty = argv[0];
		if (argc > 1) {
			strlcpy(term, "TERM=", sizeof(term));
			strlcat(term, argv[1], sizeof(term));
		}
	}

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGHUP, &sa, NULL);

	putenv(term);

	setsid();

	fd = open(tty, O_RDWR);
	if (fd < 0)
		eprintf("open %s:", tty);
	if (isatty(fd) == 0)
		eprintf("%s is not a tty\n", tty);
	if (ioctl(fd, TIOCSCTTY, (void *)1) == 0) {
		if (vhangup() < 0)
			eprintf("vhangup:");
	} else {
		fprintf(stderr, "could not set controlling tty\n");
	}
	close(fd);
	fd = open(tty, O_RDWR);
	if (fd < 0)
		eprintf("open %s:", tty);
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);

	sa.sa_handler = SIG_DFL;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGHUP, &sa, NULL);

	putchar('\n');
	printf("Login: ");
	fflush(stdout);

	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);
	memset(logname, 0, sizeof(logname));
	while (1) {
		n = read(STDIN_FILENO, &c, 1);
		if (n < 0)
			eprintf("read:");
		if (n == 0)
			return EXIT_FAILURE;
		if (i >= sizeof(logname) - 1)
			eprintf("login name too long\n");
		if (c == '\n' || c == '\r')
			break;
		logname[i++] = c;
	}
	if (logname[0] == '-')
		eprintf("login name cannot start with '-'\n");
	if (logname[0] == '\0')
		return EXIT_FAILURE;
	return execvp("/bin/login", (char *[]){ "login", logname, NULL });
}
