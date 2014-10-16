/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utmp.h>
#include <unistd.h>

#include "config.h"
#include "util.h"

void
usage(void)
{
	fputs("last [user]\n", stderr);
	exit(1);
}

int
main(int argc, char **argv)
{
	FILE *fp;
	struct utmp ut;
	char *user, *file, *prog;
	time_t t;

	switch (argc) {
	case 1:
		user = NULL;
		break;
	case 2:
		user = argv[1];
		break;
	default:
		usage();
	}

	prog = basename(argv[0]);
	file = (!strcmp(prog, "last")) ? WTMP_PATH : BTMP_PATH;
	if ((fp = fopen(file, "r")) == NULL)
                eprintf("fopen %s:", file);

	while (fread(&ut, sizeof(ut), 1, fp) == 1) {
		if (ut.ut_type != USER_PROCESS ||
		    (user && strcmp(user, ut.ut_name))) {
			continue;
		}

		t = ut.ut_time;
		printf("%-8.8s %-8.8s %-16.16s %s",
		       ut.ut_user, ut.ut_line, ut.ut_host, ctime(&t));
	}
	if (fclose(fp))
		eprintf("fclose %s:", file);
	return 0;
}
