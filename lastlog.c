/* See LICENSE file for copyright and license details. */
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utmp.h>

#define PASSWD   "/etc/passwd"

static FILE *last;

static void
lastlog(char *user)
{
	struct passwd *pwd;
	struct lastlog ll;
	time_t lltime;

	if ((pwd = getpwnam(user)) == NULL) {
		fprintf(stderr, "unknown user: %s\n", user);
		return;
	}

	fseek(last, pwd->pw_uid * sizeof(struct lastlog), 0);
	fread(&ll, sizeof(struct lastlog), 1, last);

	if (ferror(last)) {
		perror("error reading lastlog");
		exit(EXIT_FAILURE);
	}

	/* on glibc `ll_time' can be an int32_t with compat32
	 * avoid compiler warning when calling ctime() */
	lltime = ll.ll_time;
	printf("%-8.8s %-8.8s %-16.16s %s",
	       user, ll.ll_line, ll.ll_host, ctime(&lltime));
}

int
main(int argc, char **argv)
{
	FILE *fp;
	char line[512], *p;

	if ((last = fopen(_PATH_LASTLOG, "r")) == NULL) {
	    perror(_PATH_LASTLOG);
	    exit(EXIT_FAILURE);
	}

	if (argc > 1) {
		while (*++argv)
			lastlog(*argv);
	} else {
		if ((fp = fopen(PASSWD, "r")) == NULL) {
			perror(PASSWD);
			exit(EXIT_FAILURE);
		}
		while ((fgets(line, sizeof(line), fp)) != NULL) {
			if ((p = strchr(line, ':')) == NULL) {
				fputs("incorrect password file", stderr);
				exit(-1);
			}
			*p = '\0';
			lastlog(line);
		}
		if (fclose(fp))
			perror(PASSWD);
	}

	fclose(last);

	return EXIT_SUCCESS;
}
