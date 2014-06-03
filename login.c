/* See LICENSE file for copyright and license details. */
#define _XOPEN_SOURCE
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <shadow.h>
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
	struct spwd *spw;
	char *pass, *cryptpass;
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

	switch (pw->pw_passwd[0]) {
	case '!':
	case '*':
		eprintf("denied\n");
	}

	uid = pw->pw_uid;
	gid = pw->pw_gid;

	/* Empty password? Login now */
	if (pw->pw_passwd[0] == '\0')
		goto login;

	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	pass = getpass("Password: ");
	if (!pass)
		eprintf("getpass:");

	if (pw->pw_passwd[0] == 'x' && pw->pw_passwd[1] == '\0') {
		errno = 0;
		spw = getspnam(argv[0]);
		if (errno)
			eprintf("getspnam: %s:", argv[0]);
		else if (!spw)
			eprintf("who are you?\n");
		switch (spw->sp_pwdp[0]) {
		case '!':
		case '*':
			eprintf("denied\n");
		}
		cryptpass = crypt(pass, spw->sp_pwdp);
		if (!cryptpass)
			eprintf("crypt:");
		if (strcmp(cryptpass, spw->sp_pwdp) != 0)
			eprintf("login failed\n");
	} else {
		cryptpass = crypt(pass, pw->pw_passwd);
		if (!cryptpass)
			eprintf("crypt:");
		if (strcmp(cryptpass, pw->pw_passwd) != 0)
			eprintf("login failed\n");
	}

login:
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
	if (preserve == 0)
		clearenv();
	setenv("HOME", pw->pw_dir, !preserve);
	setenv("SHELL", pw->pw_shell, !preserve);
	setenv("USER", pw->pw_name, !preserve);
	setenv("LOGNAME", pw->pw_name, !preserve);
	setenv("PATH", ENV_PATH, !preserve);
	if (chdir(pw->pw_dir) < 0)
		eprintf("chdir %s:", pw->pw_dir);
	execlp(pw->pw_shell, pw->pw_shell, "-l", NULL);
	weprintf("execlp %s:", pw->pw_shell);
	return (errno == ENOENT) ? 127 : 126;
}
