/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../proc.h"
#include "../util.h"

int
proceuid(pid_t pid, uid_t *euid)
{
	FILE *fp;
	char buf[BUFSIZ], *p;
	char path[PATH_MAX];
	uid_t procuid, proceuid;

	snprintf(path, sizeof(path), "/proc/%d/status", pid);
	if (!(fp = fopen(path, "r")))
		eprintf("%s fopen:", path);
	while (fgets(buf, sizeof(buf), fp)) {
		if (!strncmp(buf, "Uid:", 4)) {
			p = buf + strlen("Uid:");
			sscanf(p, "%u %u", &procuid, &proceuid);
			*euid = proceuid;
			fclose(fp);
			return 0;
		}
	}
	fclose(fp);
	return -1;
}

int
parsestat(pid_t pid, struct procstat *ps)
{
	char path[PATH_MAX];
	FILE *fp;

	snprintf(path, sizeof(path), "/proc/%d/stat", pid);
	if (!(fp = fopen(path, "r")))
		eprintf("fopen %s:", path);
	fscanf(fp, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu",
	       &ps->pid, ps->comm,
	       &ps->state, &ps->ppid, &ps->pgrp,
	       &ps->sid, &ps->tty_nr, &ps->tpgid, &ps->flags,
	       &ps->minflt, &ps->cminflt, &ps->majflt, &ps->cmajflt,
	       &ps->utime, &ps->stime);
	fscanf(fp, "%ld %ld %ld %ld %ld %ld %lld %lu %ld %ld",
	       &ps->cutime, &ps->cstime, &ps->priority, &ps->nice,
	       &ps->num_threads, &ps->itrealvalue, &ps->starttime,
	       &ps->vsize, &ps->rss, &ps->rsslim);
	/* Filter out '(' and ')' from comm */
	ps->comm[strlen(ps->comm) - 1] = '\0';
	memmove(ps->comm, ps->comm + 1, strlen(ps->comm));
	fclose(fp);
	return 0;
}

int
validps(const char *path)
{
	char *end;

	errno = 0;
	strtol(path, &end, 10);
	if (*end != '\0')
		return 0;
	if (errno != 0)
		return 0;
	return 1;
}
