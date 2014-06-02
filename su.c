/* See LICENSE file for copyright and license details. */
#define _XOPEN_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "util.h"

extern char **environ;

static const char *randreply(void);
static char *msetenv(const char *, const char *);
static void dologin(struct passwd *);

static void
usage(void)
{
	eprintf("usage: %s [-lp] [user]\n", argv0);
}

static int lflag = 0;
static int pflag = 0;

int
main(int argc, char *argv[])
{
	char *usr = "root", *pass, *cryptpass;
	char * const *newargv;
	struct spwd *spw;
	struct passwd *pw;
	uid_t uid;

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
		;
	else if (argc == 1)
		usr = argv[0];
	else
		usage();

	srand(time(NULL));

	errno = 0;
	spw = getspnam(usr);
	if (errno)
		eprintf("getspnam: %s:", usr);
	else if (!spw)
		eprintf("who are you?\n");

	switch (spw->sp_pwdp[0]) {
	case '!':
	case '*':
		eprintf("Denied\n");
	case '$':
		break;
	default:
		eprintf("Invalid shadow record\n");
	}

	uid = getuid();
	if (uid) {
		pass = getpass("Password: ");
		if (!pass)
			eprintf("getpass:");

		cryptpass = crypt(pass, spw->sp_pwdp);
		explicit_bzero(pass, strlen(pass));
		if (!cryptpass)
			eprintf("crypt:");

		if (strcmp(cryptpass, spw->sp_pwdp) != 0)
			eprintf(randreply());
		explicit_bzero(cryptpass, strlen(cryptpass));
		explicit_bzero(spw, sizeof *spw);
	}

	errno = 0;
	pw = getpwnam(usr);
	if (errno)
		eprintf("getpwnam: %s", usr);
	else if (!pw)
		eprintf("who are you?\n");

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
			setenv("SHELL", pw->pw_shell, 1);
			if (strcmp(pw->pw_name, "root") != 0) {
				setenv("USER", pw->pw_name, 1);
				setenv("LOGNAME", pw->pw_name, 1);
			}
		}
		if (strcmp(pw->pw_name, "root") == 0)
			setenv("PATH", ENV_SUPATH, 1);
		else
			setenv("PATH", ENV_PATH, 1);
		execve(pflag ? getenv("SHELL") : pw->pw_shell,
		       newargv, environ);
	}
	return (errno == ENOENT) ? 127 : 126;
}

static const char *
randreply(void)
{
	static const char *replies[] = {
		"Time flies like an arrow, fruit flies like a banana.\n",
		"Denied.\n",
		"You type like a dairy farmer.\n",
		"CChheecckk yyoouurr dduupplleexx sswwiittcchh..\n",
		"I met a girl with 12 nipples, it sounds weird dozen tit?\n",
		"Here I am, brain the size of a planet and they ask me to keep hashing rubbish.\n",
		"Clones are people two.\n",
		"Your mom is an interesting su response.\n",
		"no.\n",
		"Your mom forgot to null-terminate???B?33??Abort (core dumped)\n",
		"A fool-proof method for sculpting an elephant: first, get a huge block of marble; then you chip away everything that doesn't look like an elephant.\n",
		"Bloating .data for fun and profit.\n",
	};
	return replies[rand() % LEN(replies)];
}

static char *
msetenv(const char *name, const char *value)
{
	char *buf;
	size_t sz;

	sz = strlen(name) + strlen(value) + 2;
	buf = emalloc(sz);
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
		msetenv("PATH",
			strcmp(pw->pw_name, "root") == 0 ?
			ENV_SUPATH : ENV_PATH),
		NULL
	};
	if (chdir(pw->pw_dir) < 0)
		eprintf("chdir %s:", pw->pw_dir);
	execve(pw->pw_shell, newargv, newenv);
}
