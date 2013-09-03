/* See LICENSE file for copyright and license details. */
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grabmntinfo.h"
#include "util.h"

struct {
	const char *opt;
	const char *notopt;
	unsigned long v;
} optnames[] = {
	{ "remount",	NULL,		MS_REMOUNT },
	{ "ro",		"rw",		MS_RDONLY },
	{ "sync",	"async",	MS_SYNCHRONOUS },
	{ "dirsync",	NULL,		MS_DIRSYNC },
	{ "nodev",	"dev",		MS_NODEV },
	{ "noatime",	"atime",	MS_NOATIME },
	{ "nodiratime",	"diratime",	MS_NODIRATIME },
	{ "noexec",	"exec",		MS_NOEXEC },
	{ "nosuid",	"suid",		MS_NOSUID },
	{ "mand",	"nomand",	MS_MANDLOCK },
	{ "relatime",	"norelatime",	MS_RELATIME },
	{ NULL,		NULL,		0 }
};

static struct option {
	char *name;
	struct option *next;
} *opthead;

static void
usage(void)
{
	eprintf("usage: %s [-BMRd] [-t fstype] [-o options] source target\n",
		argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	unsigned long flags = 0;
	char *types = NULL, *arg = NULL, *p;
	const char *source;
	const char *target;
	struct stat st1, st2;
	int validopt;
	void *data = NULL;
	struct mntinfo *minfo = NULL;
	struct option *opt, *tmp;
	int siz;
	int oflag = 0;

	ARGBEGIN {
	case 'B':
		flags |= MS_BIND;
		break;
	case 'M':
		flags |= MS_MOVE;
		break;
	case 'R':
		flags |= MS_REC;
		break;
	case 'd':
		data = EARGF(usage());
		break;
	case 'o':
		oflag = 1;
		arg = EARGF(usage());
		for (p = strtok(arg, ","); p; p = strtok(NULL, ",")) {
			opt = malloc(sizeof(*opt));
			if (!opt)
				eprintf("malloc:");
			opt->name = strdup(p);
			if (!opt->name)
				eprintf("strdup:");
			opt->next = opthead;
			opthead = opt;
		}
		break;
	case 't':
		types = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (opt = opthead; opt; opt = opt->next) {
		validopt = 0;
		for (i = 0; optnames[i].v; i++) {
			if (optnames[i].opt) {
				if (!strcmp(opt->name,
					    optnames[i].opt)) {
					flags |= optnames[i].v;
					validopt = 1;
					break;
				}
			}
			if (optnames[i].notopt) {
				if (!strcmp(opt->name,
					    optnames[i].notopt)) {
					flags &= ~optnames[i].v;
					validopt = 1;
					break;
				}
			}
		}
		if (!validopt)
			break;
	}
	if (oflag && !validopt)
		enprintf(1, "unknown option: %s\n", opt->name);

	source = argv[0];
	target = argv[1];

	if (!target) {
		target = argv[0];
		source = NULL;
		if (stat(target, &st1) < 0)
			eprintf("stat %s:", target);
		siz = grabmntinfo(&minfo);
		if (!siz)
			eprintf("grabmntinfo:");
		for (i = 0; i < siz; i++) {
			if (stat(minfo[i].mntdir, &st2) < 0)
				eprintf("stat %s:", minfo[i].mntdir);
			if (st1.st_dev == st2.st_dev &&
			    st1.st_ino == st2.st_ino) {
				source = minfo[i].fsname;
				break;
			}
		}
		if (!source)
			enprintf(1, "can't find %s mountpoint\n",
				 target);
	}

	if (mount(source, target, types, flags, data) < 0)
		eprintf("mount:");

	free(minfo);

	opt = opthead;
	while (opt) {
		tmp = opt->next;
		free(opt->name);
		free(opt);
		opt = tmp;
	}

	return 0;
}
