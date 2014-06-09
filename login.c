/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include "passwd.h"
#include "config.h"
#include "util.h"

static int dologin(struct passwd *, int);

static void
usage(void)
{
	eprintf("usage: %s [-p] username\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct passwd *pw;
	char *pass;
	uid_t uid;
	gid_t gid;
	int pflag = 0;

	ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	if (isatty(STDIN_FILENO) == 0)
		eprintf("stdin is not a tty\n");

	errno = 0;
	pw = getpwnam(argv[0]);
	if (errno)
		eprintf("getpwnam: %s:", argv[0]);
	else if (!pw)
		eprintf("who are you?\n");

	uid = pw->pw_uid;
	gid = pw->pw_gid;

	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	pass = getpass("Password: "); putchar('\n');
	if (!pass)
		eprintf("getpass:");
	if (pw_check(pw, pass) <= 0)
		exit(EXIT_FAILURE);

	if (initgroups(argv[0], gid) < 0)
		eprintf("initgroups:");
	if (setgid(gid) < 0)
		eprintf("setgid:");
	if (setuid(uid) < 0)
		eprintf("setuid:");

	return dologin(pw, pflag);
}

static int
dologin(struct passwd *pw, int preserve)
{
	char *shell = pw->pw_shell[0] == '\0' ? "/bin/sh" : pw->pw_shell;

	if (preserve == 0)
		clearenv();
	setenv("HOME", pw->pw_dir, 1);
	setenv("SHELL", shell, 1);
	setenv("USER", pw->pw_name, 1);
	setenv("LOGNAME", pw->pw_name, 1);
	setenv("PATH", ENV_PATH, 1);
	if (chdir(pw->pw_dir) < 0)
		eprintf("chdir %s:", pw->pw_dir);
	execlp(shell, shell, "-l", NULL);
	weprintf("execlp %s:", shell);
	return (errno == ENOENT) ? 127 : 126;
}
