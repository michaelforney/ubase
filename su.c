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

extern char **environ;

static char *msetenv(const char *, const char *);
static void dologin(struct passwd *);

static void
usage(void)
{
	eprintf("usage: %s [-lp] [username]\n", argv0);
}

static int lflag = 0;
static int pflag = 0;

int
main(int argc, char **argv)
{
	char *usr, *pass, *cryptpass;
	char * const *newargv;
	struct spwd *spw;
	struct passwd *pw;
	uid_t uid;
	int i;

	ARGBEGIN {
	case 'l':
		lflag = 1;
		break;
	case 'p':
		pflag = 1;
		break;
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

	errno = 0;
	spw = getspnam(usr);
	if (errno)
		eprintf("getspnam: %s:", usr);
	else if (!spw)
		enprintf(EXIT_FAILURE, "who are you?\n");

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

		cryptpass = crypt(pass, spw->sp_pwdp);
		for (i = 0; pass[i]; i++)
			pass[i] = '\0';
		if (!cryptpass)
			eprintf("crypt:");

		if (strcmp(cryptpass, spw->sp_pwdp) != 0)
			enprintf(EXIT_FAILURE, "Denied\n");
	}

	errno = 0;
	pw = getpwnam(usr);
	if (errno)
		eprintf("getpwnam: %s", usr);
	else if (!pw)
		enprintf(EXIT_FAILURE, "who are you?\n");

	if (initgroups(usr, pw->pw_gid) < 0)
		eprintf("initgroups:");
	if (setgid(pw->pw_gid) < 0)
		eprintf("setgid:");
	if (setuid(pw->pw_uid) < 0)
		eprintf("setuid:");

	if (lflag) {
		dologin(pw);
	} else {
		newargv = (char *const[]){pw->pw_shell, NULL};
		if (!pflag) {
			setenv("HOME", pw->pw_dir, 1);
			setenv("SHELL", pw->pw_dir, 1);
			if (strcmp(pw->pw_name, "root") != 0) {
				setenv("USER", pw->pw_name, 1);
				setenv("LOGNAME", pw->pw_name, 1);
			}
		}
		execve(pflag ? getenv("SHELL") : pw->pw_shell,
		       newargv, environ);
	}
	return (errno == ENOENT) ? 127 : 126;
}

static char *
msetenv(const char *name, const char *value)
{
	char *buf;
	size_t sz;

	sz = strlen(name) + strlen(value) + 2;
	buf = malloc(sz);
	if (!buf)
		eprintf("malloc:");
	snprintf(buf, sz, "%s=%s", name, value);
	return buf;
}

static void
dologin(struct passwd *pw)
{
	char shbuf[strlen(pw->pw_shell) + 1];
	char * const *newargv;
	char * const *newenv;

	strcpy(shbuf, pw->pw_shell);
	newargv = (char *const[]){shbuf, NULL};
	newenv = (char *const[]){
		msetenv("HOME", pw->pw_dir),
		msetenv("SHELL", pw->pw_shell),
		msetenv("USER", pw->pw_name),
		msetenv("LOGNAME", pw->pw_name),
		msetenv("TERM", getenv("TERM")),
		msetenv("PATH", getenv("PATH")),
		NULL
	};
	if (chdir(pw->pw_dir) < 0)
		eprintf("chdir %s:", pw->pw_dir);
	execve(pw->pw_shell, newargv, newenv);
}
