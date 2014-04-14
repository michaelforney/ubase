/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <linux/magic.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <dirent.h>
#include "util.h"

static void
delete_content(const char *dir, dev_t curdevice)
{
	char path[PATH_MAX];
	DIR *d;
	struct stat st;
	struct dirent *dent;

	/* don't dive into other filesystems */
	if (lstat(dir, &st) || st.st_dev != curdevice)
		return;
	/* delete contents recursively */
	if (S_ISDIR(st.st_mode)) {
		d = opendir(dir);
		if (d) {
			for(; (dent = readdir(d)) ;) {
				/* skip ".." and "." */
				if (dent->d_name[0] == '.'
				 && ((dent->d_name[1] == '.' && dent->d_name[2] == 0)
				     || (dent->d_name[1] == 0)))
					continue;

				/* build path and dive deeper */
				strlcat(path, dir, sizeof(path));
				strlcat(path, dent->d_name, sizeof(path));

				delete_content(path, curdevice);
				path[0] = 0;
			}
			closedir(d);

			/* remove now empty dir */
			rmdir(dir);
		}
	} else {
		/* unlink non-directory */
		unlink(dir);
	}
}

static void
usage(void)
{
	eprintf("usage: %s [-c console] [newroot] [init] (PID 1)\n", argv0);
}

int
main(int argc, char **argv)
{
	char *console = NULL;
	dev_t curdev;
	struct stat st;
	struct statfs stfs;

	ARGBEGIN {
	case 'c':
		console = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	/* check number of args and if we are PID 1 */
	if (argc != 2 || getpid() != 1)
		usage();

	/* chdir to newroot and make sure it's a different fs */
	if (chdir(argv[0]))
		eprintf("chdir %s:", argv[0]);

	if (stat("/", &st))
		eprintf("stat %s:", "/");

	curdev = st.st_dev;
	if (stat(".", &st))
		eprintf("stat %s:", ".");
	if (st.st_dev == curdev)
		usage();

	/* avoids trouble with real filesystems */
	if (stat("/init", &st) || !S_ISREG(st.st_mode))
		eprintf("/init is not a regular file\n");

	statfs("/", &stfs);
	if ((unsigned)stfs.f_type != RAMFS_MAGIC && (unsigned)stfs.f_type != TMPFS_MAGIC)
		eprintf("current filesystem is not a RAMFS or TMPFS\n");

	/* wipe / */
	delete_content("/", curdev);

	/* overmount / with newroot and chroot into it */
	if (mount(".", "/", NULL, MS_MOVE, NULL))
		eprintf("mount %s:", ".");

	if (chroot("."))
		eprintf("chroot failed\n");

	/* if -c is set, redirect stdin/stdout/stderr to console */
	if (console) {
		close(0);
		if(open(console, O_RDWR) == -1){
			eprintf("open %s:", console);
		}
		if (dup2(0,1) == -1){
			eprintf("dup2 %s:", "0,1");
		}
		if (dup2(0,2) == -1){
			eprintf("dup2 %s:", "0,2");
		}
	}

	/* execute init */
	execv(argv[1], argv);
	eprintf("can't execute '%s'\n", argv[1]);
	return EXIT_FAILURE;
}
