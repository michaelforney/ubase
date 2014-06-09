/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "passwd.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s username\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *pass;
	char *cryptpass1 = NULL, *cryptpass2 = NULL, *cryptpass3 = NULL;
	char *p;
	char template[] = "/tmp/pw.XXXXXX";
	uid_t uid;
	struct passwd *pw;
	int ffd, tfd;
	int r;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	errno = 0;
	pw = getpwnam(argv[0]);
	if (errno)
		eprintf("getpwnam: %s:", argv[0]);
	else if (!pw)
		eprintf("who are you?\n");

	if (pw->pw_passwd[0] == 'x' && pw->pw_passwd[1] == '\0')
		eprintf("no shadow support\n");

	uid = getuid();
	if (uid == 0) {
		if (pw->pw_passwd[0] == '!' ||
		    pw->pw_passwd[0] == '*' ||
		    pw->pw_passwd[0] == '\0')
			pw->pw_passwd = "$6$";
		goto newpass;
	} else {
		if (pw->pw_passwd[0] == '!' ||
		    pw->pw_passwd[0] == '*')
			eprintf("denied\n");
		if (pw->pw_passwd[0] == '\0') {
			pw->pw_passwd = "$6$";
			goto newpass;
		}
	}

	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	pass = getpass("Current password: ");
	putchar('\n');
	if (!pass)
		eprintf("getpass:");
	if (pass[0] == '\0')
		eprintf("no password supplied\n");
	p = crypt(pass, pw->pw_passwd);
	if (!p)
		eprintf("crypt:");
	cryptpass1 = estrdup(p);
	if (strcmp(cryptpass1, pw->pw_passwd) != 0)
		eprintf("incorrect password\n");

newpass:
	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	pass = getpass("Enter new password: ");
	putchar('\n');
	if (!pass)
		eprintf("getpass:");
	if (pass[0] == '\0')
		eprintf("no password supplied\n");
	p = crypt(pass, pw->pw_passwd);
	if (!p)
		eprintf("crypt:");
	cryptpass2 = estrdup(p);
	if (cryptpass1 && strcmp(cryptpass1, cryptpass2) == 0)
		eprintf("password left unchanged\n");

	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	pass = getpass("Retype new password: ");
	putchar('\n');
	if (!pass)
		eprintf("getpass:");
	if (pass[0] == '\0')
		eprintf("no password supplied\n");
	p = crypt(pass, pw->pw_passwd);
	if (!p)
		eprintf("crypt:");
	cryptpass3 = estrdup(p);
	if (strcmp(cryptpass2, cryptpass3) != 0)
		eprintf("passwords don't match\n");

	pw->pw_passwd = cryptpass3;

	ffd = open("/etc/passwd", O_RDWR);
	if (ffd < 0)
		eprintf("open %s:", "/etc/passwd");

	tfd = mkostemp(template, O_RDWR);
	if (tfd < 0)
		eprintf("mkstemp:");

	r = pw_copy(ffd, tfd, pw);
	if (r < 0) {
		unlink(template);
		exit(EXIT_FAILURE);
	}

	r = lseek(ffd, 0, SEEK_SET);
	if (r < 0) {
		unlink(template);
		exit(EXIT_FAILURE);
	}
	r = lseek(tfd, 0, SEEK_SET);
	if (r < 0) {
		unlink(template);
		exit(EXIT_FAILURE);
	}

	r = pw_copy(tfd, ffd, NULL);
	if (r < 0) {
		unlink(template);
		exit(EXIT_FAILURE);
	}

	close(tfd);
	close(ffd);
	unlink(template);
	free(cryptpass3);
	free(cryptpass2);
	free(cryptpass1);

	return EXIT_SUCCESS;
}
