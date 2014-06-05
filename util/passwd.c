/* See LICENSE file for copyright and license details. */
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../text.h"
#include "../util.h"

int
pw_scan(char *bp, struct passwd *pw)
{
	char *p;

	memset(pw, 0, sizeof(*pw));

	/* login name */
	p = strsep(&bp, ":");
	if (!p || *p == '\0')
		goto corrupt;
	pw->pw_name = p;

	/* passwd */
	p = strsep(&bp, ":");
	if (!p)
		goto corrupt;
	pw->pw_passwd = p;

	/* uid */
	p = strsep(&bp, ":");
	if (!p)
		goto corrupt;
	pw->pw_uid = estrtol(p, 10);

	/* gid */
	p = strsep(&bp, ":");
	if (!p)
		goto corrupt;
	pw->pw_gid = estrtol(p, 10);

	/* user name or comment */
	p = strsep(&bp, ":");
	if (!p)
		goto corrupt;
	pw->pw_gecos = p;

	/* home directory */
	p = strsep(&bp, ":");
	if (!p)
		goto corrupt;
	pw->pw_dir = p;

	/* optional shell */
	p = strsep(&bp, ":");
	if (!p)
		goto corrupt;
	pw->pw_shell = p;

	/* look for redundant entries */
	p = strsep(&bp, ":");
	if (p)
		goto corrupt;

	return 0;
corrupt:
	weprintf("corrupted passwd entry\n");
	return -1;
}

int
pw_copy(int ffd, int tfd, const struct passwd *newpw)
{
	struct passwd pw;
	char *buf = NULL, *p;
	size_t size = 0;
	FILE *from, *to;

	from = fdopen(ffd, "r");
	if (!from) {
		weprintf("fdopen:");
		return -1;
	}
	to = fdopen(tfd, "w");
	if (!to) {
		weprintf("fdopen:");
		return -1;
	}
	while (agetline(&buf, &size, from) != -1) {
		p = strdup(buf);
		if (!p) {
			weprintf("strdup:");
			return -1;
		}
		if (newpw) {
			if (pw_scan(p, &pw) < 0)
				return -1;
			if (strcmp(pw.pw_name, newpw->pw_name) == 0) {
				fprintf(to, "%s:%s:%u:%u:%s:%s:%s\n",
					newpw->pw_name,
					newpw->pw_passwd,
					newpw->pw_uid,
					newpw->pw_gid,
					newpw->pw_gecos,
					newpw->pw_dir,
					newpw->pw_shell);
					continue;
			}
		}
		fprintf(to, "%s", buf);
		free(p);
	}
	fflush(to);
	return 0;
}
