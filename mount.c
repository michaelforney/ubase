/* See LICENSE file for copyright and license details. */
#include <sys/mount.h>
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
	char *types = NULL, *opt = NULL, *p;
	const char *source;
	const char *target;
	int validopt;
	void *data = NULL;
	struct mntinfo *minfo = NULL;
	int siz;

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
		opt = EARGF(usage());
		p = strtok(opt, ",");
		while (p) {
			validopt = 0;
			for (i = 0; optnames[i].v; i++) {
				if (optnames[i].opt) {
					if (!strcmp(p, optnames[i].opt)) {
						flags |= optnames[i].v;
						validopt = 1;
						break;
					}
				}
				if (optnames[i].notopt) {
					if (!strcmp(p, optnames[i].notopt)) {
						flags &= ~optnames[i].v;
						validopt = 1;
						break;
					}
				}
			}
			if (!validopt)
				break;
			p = strtok(NULL, ",");
		}
		if (!validopt)
			enprintf(1, "unknown option: %s\n", p);
		break;
	case 't':
		types = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	source = argv[0];
	target = argv[1];

	if (!target) {
		target = source;
		source = NULL;
		siz = grabmntinfo(&minfo);
		if (!siz)
			eprintf("grabmntinfo:");
		for (i = 0; i < siz; i++)
			if (!strcmp(minfo[i].mntdir, target))
				source = minfo[i].fsname;
		if (!source)
			enprintf(1, "can't find %s mountpoint\n",
				 target);
	}

	if (mount(source, target, types, flags, data) < 0)
		eprintf("mount:");

	free(minfo);

	return 0;
}
