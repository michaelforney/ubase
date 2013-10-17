/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [username]\n", argv0);
}

int
main(int argc, char **argv)
{
	char *usr, *pass, *cryptpass;
	char **newargv;
	struct spwd *spw;
	struct passwd *pw;
	uid_t uid;
	int i;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usr = "root";
	else if (argc == 1)
		usr = argv[0];
	else
		usage();

	uid = getuid();

	spw = getspnam(usr);
	if (!spw)
		eprintf("getspnam: %s:", usr);

	switch (spw->sp_pwdp[0]) {
	case '!':
	case '*':
		enprintf(EXIT_FAILURE, "Denied\n");
	case '$':
		break;
	default:
		enprintf(EXIT_FAILURE, "Invalid shadow record\n");
	}

	if (uid) {
		pass = getpass("Password: ");
		if (!pass)
			eprintf("getpass:");
	}

	cryptpass = crypt(pass, spw->sp_pwdp);
	for (i = 0; pass[i]; i++)
		pass[i] = '\0';
	if (!cryptpass)
		eprintf("crypt:");

	if (strcmp(cryptpass, spw->sp_pwdp) != 0)
		enprintf(EXIT_FAILURE, "Denied\n");

	errno = 0;
	pw = getpwnam(usr);
	if (errno)
		eprintf("getpwnam: %s", usr);
	else if (!pw)
		enprintf(EXIT_FAILURE, "getpwnam: %s: no such user\n", usr);

	if (initgroups(usr, pw->pw_gid) < 0)
		eprintf("initgroups:");
	if (setgid(pw->pw_gid) < 0)
		eprintf("setgid:");
	if (setuid(pw->pw_uid) < 0)
		eprintf("setuid:");

	newargv = malloc(2 * sizeof(char *));
	if (!newargv)
		eprintf("malloc:");
	newargv[0] = pw->pw_shell;
	newargv[1] = NULL;
	setenv("HOME", pw->pw_dir, 1);
	execve(pw->pw_shell, newargv, environ);
	return (errno == ENOENT) ? 127 : 126;
}
