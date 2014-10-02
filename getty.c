/* See LICENSE file for copyright and license details. */
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>

#include "config.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [tty] [term] [cmd] [args...]\n", argv0);
}

static char *tty = "/dev/tty1";
static char *defaultterm = "linux";

int
main(int argc, char *argv[])
{
	char term[128], logname[LOGIN_NAME_MAX], c;
	char hostname[HOST_NAME_MAX + 1];
	struct utmp usr;
	struct sigaction sa;
	FILE *fp;
	int fd;
	unsigned int i = 0;
	ssize_t n;
	long pos;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	strlcpy(term, defaultterm, sizeof(term));
	if (argc > 0) {
		tty = argv[0];
		if (argc > 1)
			strlcpy(term, argv[1], sizeof(term));
	}

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGHUP, &sa, NULL);

	setenv("TERM", term, 1);

	setsid();

	fd = open(tty, O_RDWR);
	if (fd < 0)
		eprintf("open %s:", tty);
	if (isatty(fd) == 0)
		eprintf("%s is not a tty\n", tty);
	if (ioctl(fd, TIOCSCTTY, (void *)1) == 0)
		vhangup();
	else
		weprintf("TIOCSCTTY: could not set controlling tty\n");

	close(fd);
	fd = open(tty, O_RDWR);
	if (fd < 0)
		eprintf("open %s:", tty);
	if (dup2(fd, STDIN_FILENO) != STDIN_FILENO)
		eprintf("dup2:");
	if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO)
		eprintf("dup2:");
	if (dup2(fd, STDERR_FILENO) != STDERR_FILENO)
		eprintf("dup2:");

	sa.sa_handler = SIG_DFL;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGHUP, &sa, NULL);

	if (fchown(fd, 0, 0) < 0)
		eprintf("fchown %s:", tty);
	if (fchmod(fd, 0600) < 0)
		eprintf("chmod %s:", tty);

	/* Clear all utmp entries for this tty */
	fp = fopen(UTMP_PATH, "r+");
	if (fp) {
		do {
			pos = ftell(fp);
			if (fread(&usr, sizeof(usr), 1, fp) != 1)
				break;
			if (usr.ut_line[0] == '\0')
				continue;
			if (strcmp(usr.ut_line, tty) != 0)
				continue;
			memset(&usr, 0, sizeof(usr));
			fseek(fp, pos, SEEK_SET);
			if (fwrite(&usr, sizeof(usr), 1, fp) != 1)
				break;
		} while (1);
		if (ferror(fp))
			weprintf("%s: I/O error:", UTMP_PATH);
		fclose(fp);
	}

	if (argc > 2)
		return execvp(argv[2], argv + 2);

	if (gethostname(hostname, sizeof(hostname)) == 0)
		printf("%s ", hostname);
	printf("login: ");
	fflush(stdout);

	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);
	memset(logname, 0, sizeof(logname));
	while (1) {
		n = read(STDIN_FILENO, &c, 1);
		if (n < 0)
			eprintf("read:");
		if (n == 0)
			return 1;
		if (i >= sizeof(logname) - 1)
			eprintf("login name too long\n");
		if (c == '\n' || c == '\r')
			break;
		logname[i++] = c;
	}
	if (logname[0] == '-')
		eprintf("login name cannot start with '-'\n");
	if (logname[0] == '\0')
		return 1;
	return execlp("/bin/login", "login", "-p", logname, NULL);
}
