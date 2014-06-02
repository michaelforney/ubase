/* See LICENSE file for copyright and license details. */
#define _XOPEN_SOURCE
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
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
	char *pass, *cryptpass;
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

	errno = 0;
	pw = getpwnam(argv[0]);
	if (errno)
		eprintf("getpwnam: %s:", argv[0]);
	else if (!pw)
		eprintf("who are you?\n");

	switch (pw->pw_passwd[0]) {
	case '!':
	case '*':
		eprintf("denied\n");
	}

	if (pw->pw_passwd[0] == 'x' && pw->pw_passwd[1] == '\0')
		eprintf("no shadow support\n");

	/* Empty password? Login now */
	if (pw->pw_passwd[0] == '\0')
		goto login;

	/* Flush pending input */
        ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	pass = getpass("Password: ");
	putchar('\n');
	if (!pass)
		eprintf("getpass:");
	cryptpass = crypt(pass, pw->pw_passwd);
	explicit_bzero(pass, strlen(pass));
	if (!cryptpass)
		eprintf("crypt:");
	if (strcmp(cryptpass, pw->pw_passwd) != 0)
		eprintf("login failed\n");

login:
	if (initgroups(argv[0], pw->pw_gid) < 0)
		eprintf("initgroups:");
	if (setgid(pw->pw_gid) < 0)
		eprintf("setgid:");
	if (setuid(pw->pw_uid) < 0)
		eprintf("setuid:");

	return dologin(pw, pflag);
}

static int
dologin(struct passwd *pw, int preserve)
{
	if (preserve == 0)
		clearenv();
	setenv("HOME", pw->pw_dir, 1);
	setenv("SHELL", pw->pw_shell, 1);
	setenv("USER", pw->pw_name, 1);
	setenv("LOGNAME", pw->pw_name, 1);
	setenv("PATH", strcmp(pw->pw_name, "root") == 0 ?
	       ENV_SUPATH : ENV_PATH, 1);
	if (chdir(pw->pw_dir) < 0)
		eprintf("chdir %s:", pw->pw_dir);
	execlp(pw->pw_shell, pw->pw_shell, "-l", NULL);
	weprintf("execvp %s:", pw->pw_shell);
	return (errno == ENOENT) ? 127 : 126;
}
