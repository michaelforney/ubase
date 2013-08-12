/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s filename...\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct stat st;
	char buf[100];
	int i, r, ret = 0;


	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (i = 0; i < argc; i++) {
		r = stat(argv[i], &st);
		if (r == -1) {
			fprintf(stderr, "stat '%s': ", argv[i]);
			perror(NULL);
			ret = 1;
			continue;
		}

		printf("  File: ‘%s’\n", argv[i]);
		printf("  Size: %ju\tBlocks: %ju\tIO Block: %ju\n", (uintmax_t)st.st_size,
		       (uintmax_t)st.st_blocks, (uintmax_t)st.st_blksize);
		printf("Device: %xh/%ud\tInode: %ju\tLinks %ju\n", major(st.st_dev),
		       minor(st.st_dev), (uintmax_t)st.st_ino, (uintmax_t)st.st_nlink);
		printf("Access: %04o\tUid: %u\tGid: %u\n", st.st_mode & 0777, st.st_uid, st.st_gid);
		strftime(buf, sizeof(buf), "%F %T %z", localtime(&st.st_atime));
		printf("Access: %s\n", buf);
		strftime(buf, sizeof(buf), "%F %T %z", localtime(&st.st_mtime));
		printf("Modify: %s\n", buf);
		strftime(buf, sizeof(buf), "%F %T %z", localtime(&st.st_ctime));
		printf("Change: %s\n", buf);
	}

	return ret;
}
