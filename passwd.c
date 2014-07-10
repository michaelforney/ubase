/* See LICENSE file for copyright and license details. */
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <limits.h>
#include <shadow.h>

#include "config.h"
#include "passwd.h"
#include "util.h"
#include "text.h"

static void
usage(void)
{
	eprintf("usage: %s [username]\n", argv0);
}

static int
gettempfile(char *template)
{
	int fd;

	umask(077);
	fd = mkostemp(template, O_RDWR);
	if (fd < 0)
		weprintf("mkstemp:");
	return fd;
}

int
main(int argc, char *argv[])
{
	char *cryptpass1 = NULL, *cryptpass2 = NULL, *cryptpass3 = NULL;
	char shadowfile[PATH_MAX], *inpass, *p, *pwd = NULL;
	char template[] = "/tmp/pw.XXXXXX";
	struct passwd *pw;
	struct spwd *spw = NULL, *spwent;
	uid_t uid;
	FILE *fp = NULL, *tfp = NULL;
	int ffd = -1, tfd = -1, r, status = EXIT_FAILURE;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	pw_init();

	errno = 0;
	if (argc == 0)
		pw = getpwuid(getuid());
	else
		pw = getpwnam(argv[0]);
	if (!pw) {
		if (errno)
			eprintf("getpwnam: %s:", argv[0]);
		else
			eprintf("who are you?\n");
	}

	/* is using shadow entry ? */
	if (pw->pw_passwd[0] == 'x') {
		errno = 0;
		spw = getspnam(pw->pw_name);
		if (!spw) {
			if (errno)
				eprintf("getspnam: %s:", pw->pw_name);
			else
				eprintf("who are you?\n");
		}
		pwd = spw->sp_pwdp;
	} else {
		pwd = pw->pw_passwd;
	}

	uid = getuid();
	if (uid == 0) {
		if (pw->pw_passwd[0] == '!' ||
		    pw->pw_passwd[0] == 'x' ||
		    pw->pw_passwd[0] == '*' ||
		    pw->pw_passwd[0] == '\0')
			pw->pw_passwd = PW_CIPHER;
		goto newpass;
	} else {
		if (pw->pw_passwd[0] == '!' ||
		    pw->pw_passwd[0] == '*')
			eprintf("denied\n");
		if (pw->pw_passwd[0] == '\0') {
			pw->pw_passwd = PW_CIPHER;
			goto newpass;
		}
	}

	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	printf("Changing password for %s\n", pw->pw_name);
	inpass = getpass("Old password: ");
	if (!inpass)
		eprintf("getpass:");
	if (inpass[0] == '\0')
		eprintf("no password supplied\n");
	p = crypt(inpass, pwd);
	if (!p)
		eprintf("crypt:");
	cryptpass1 = estrdup(p);
	if (strcmp(cryptpass1, pwd) != 0)
		eprintf("incorrect password\n");

newpass:
	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	inpass = getpass("Enter new password: ");
	if (!inpass)
		eprintf("getpass:");
	if (inpass[0] == '\0')
		eprintf("no password supplied\n");
	p = crypt(inpass, pwd);
	if (!p)
		eprintf("crypt:");
	cryptpass2 = estrdup(p);
	if (cryptpass1 && strcmp(cryptpass1, cryptpass2) == 0)
		eprintf("password left unchanged\n");

	/* Flush pending input */
	ioctl(STDIN_FILENO, TCFLSH, (void *)0);

	inpass = getpass("Retype new password: ");
	if (!inpass)
		eprintf("getpass:");
	if (inpass[0] == '\0')
		eprintf("no password supplied\n");
	p = crypt(inpass, pwd);
	if (!p)
		eprintf("crypt:");
	cryptpass3 = estrdup(p);
	if (strcmp(cryptpass2, cryptpass3) != 0)
		eprintf("passwords don't match\n");

	r = snprintf(shadowfile, sizeof(shadowfile), "/etc/tcb/%s/shadow", pw->pw_name);
	if (r < 0 || (size_t)r >= sizeof(shadowfile))
		eprintf("snprintf:");
	fp = fopen(shadowfile, "r+");
	if (!fp) {
		strlcpy(shadowfile, "/etc/shadow", sizeof(shadowfile));
		fp = fopen(shadowfile, "r+");
	}
	if (fp) {
		if ((tfd = gettempfile(template)) == -1)
			goto cleanup;

		/* write to (tcb) shadow file. */
		if (!(tfp = fdopen(tfd, "w+"))) {
			weprintf("fdopen:");
			goto cleanup;
		}
		while ((spwent = fgetspent(fp))) {
			/* update entry on name match */
			if (strcmp(spwent->sp_namp, spw->sp_namp) == 0)
				spwent->sp_pwdp = cryptpass3;
			errno = 0;
			if (putspent(spwent, tfp) == -1) {
				weprintf("putspent:");
				goto cleanup;
			}
		}
		fflush(tfp);

		if (fseek(fp, 0, SEEK_SET) == -1 || fseek(tfp, 0, SEEK_SET) == -1) {
			weprintf("rewind:");
			goto cleanup;
		}

		/* old shadow file with temporary file data. */
		concat(tfp, template, fp, shadowfile);
		ftruncate(tfd, ftell(tfp));
	} else {
		/* write to /etc/passwd file. */
		ffd = open("/etc/passwd", O_RDWR);
		if (ffd < 0) {
			weprintf("open %s:", "/etc/passwd");
			goto cleanup;
		}
		pw->pw_passwd = cryptpass3;

		if ((tfd = gettempfile(template)) == -1)
			goto cleanup;

		r = pw_copy(ffd, tfd, pw);
		if (r < 0)
			goto cleanup;
		r = lseek(ffd, 0, SEEK_SET);
		if (r < 0)
			goto cleanup;
		r = lseek(tfd, 0, SEEK_SET);
		if (r < 0)
			goto cleanup;
		r = pw_copy(tfd, ffd, NULL);
		if (r < 0)
			goto cleanup;
		close(ffd);
	}
	status = EXIT_SUCCESS;

cleanup:
	if (fp)
		fclose(fp);
	if (tfp)
		fclose(tfp);
	if (tfd != -1) {
		close(tfd);
		unlink(template);
	}
	if (ffd != -1)
		close(ffd);
	free(cryptpass3);
	free(cryptpass2);
	free(cryptpass1);

	return status;
}
