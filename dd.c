/* (C) 2011-2012 Sebastian Krahmer all rights reserved.
 *
 * Optimized dd, to speed up backups etc.
 *
 * Permission has been granted to release this code under MIT/X.
 * The original code is at https://github.com/stealth/odd.  This
 * version of the code has been modified by sin@2f30.org.
 */
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <time.h>
#include <unistd.h>
#include "util.h"

struct dd_config {
	const char *in, *out;
	uint64_t skip, seek, count, b_in, b_out, rec_in, rec_out;
	off_t fsize;
	blksize_t bs;
	char quiet, nosync, direct;
	int saved_errno;
	time_t t_start, t_end;
};

static int sigint = 0;

static int
prepare_copy(struct dd_config *ddc, int *ifd, int *ofd)
{
	struct stat st;
	int fli = O_RDONLY|O_LARGEFILE|O_NOCTTY, flo = O_WRONLY|O_LARGEFILE|O_NOATIME|O_NOCTTY;
	uid_t euid = 0;
	long pagesize;

	if (ddc->direct) {
		fli |= O_DIRECT;
		flo |= O_DIRECT;
	}

	if (stat(ddc->in, &st) < 0) {
		ddc->saved_errno = errno;
		return -1;
	}

	euid = geteuid();

	if (!euid || st.st_uid == euid)
		fli |= O_NOATIME;

	if ((*ifd = open(ddc->in, fli)) < 0) {
		ddc->saved_errno = errno;
		return -1;
	}

	ddc->fsize = st.st_size;

	/* If "bsize" is not given, use optimum of both FS' */
	if (!ddc->bs) {
		struct statfs fst;
		memset(&fst, 0, sizeof(fst));
		pagesize = sysconf(_SC_PAGESIZE);
		if (pagesize <= 0)
			pagesize = 0x1000;
		if (statfs(ddc->out, &fst) < 0 || fst.f_bsize == 0)
			fst.f_bsize = pagesize;
		if ((unsigned long)fst.f_bsize > (unsigned long)st.st_blksize)
			ddc->bs = fst.f_bsize;
		else
			ddc->bs = st.st_blksize;
		if (ddc->bs == 0)
			ddc->bs = pagesize;
	}

	/* check if device or regular file */
	if (!S_ISREG(st.st_mode)) {
		if (S_ISBLK(st.st_mode)) {
			if (ioctl(*ifd, BLKGETSIZE64, &ddc->fsize) < 0) {
				ddc->saved_errno = errno;
				close(*ifd);
				return -1;
			}
		} else {
			ddc->fsize = (off_t)-1;
			if (ddc->count)
				ddc->fsize = ddc->count*ddc->bs;
		}
	}

	/* skip and seek are in block items */
	ddc->skip *= ddc->bs;
	ddc->seek *= ddc->bs;

	/* skip more bytes than are inside source file? */
	if (ddc->fsize != (off_t)-1 && ddc->skip >= (uint64_t)ddc->fsize) {
		ddc->saved_errno = EINVAL;
		close(*ifd);
		return -1;
	}

	if (!ddc->seek)
		flo |= O_CREAT|O_TRUNC;

	if ((*ofd = open(ddc->out, flo, st.st_mode)) < 0) {
		ddc->saved_errno = errno;
		close(*ifd);
		return -1;
	}

	lseek(*ifd, ddc->skip, SEEK_SET);
	lseek(*ofd, ddc->seek, SEEK_SET);
	posix_fadvise(*ifd, ddc->skip, 0, POSIX_FADV_SEQUENTIAL);
	posix_fadvise(*ofd, 0, 0, POSIX_FADV_DONTNEED);

	/* count is in block items too */
	ddc->count *= ddc->bs;

	/* If no count is given, its the filesize minus skip offset */
	if (ddc->count == 0)
		ddc->count = ddc->fsize - ddc->skip;

	return 0;
}

static int
copy_splice(struct dd_config *ddc)
{
        int ifd, ofd, p[2] = {-1, -1};
        ssize_t r = 0;
        size_t n = 0;
        if (prepare_copy(ddc, &ifd, &ofd) < 0)
                return -1;
        if (pipe(p) < 0) {
                ddc->saved_errno = errno;
                close(ifd); close(ofd);
                close(p[0]); close(p[1]);
                return -1;
        }
#ifdef F_SETPIPE_SZ
        for (n = 29; n >= 20; --n) {
                if (fcntl(p[0], F_SETPIPE_SZ, 1<<n) != -1)
                        break;
        }
#endif
        n = ddc->bs;
        for (;ddc->b_out != ddc->count && !sigint;) {
                if (n > ddc->count - ddc->b_out)
                        n = ddc->count - ddc->b_out;
                r = splice(ifd, NULL, p[1], NULL, n, SPLICE_F_MORE);
                if (r <= 0) {
                        ddc->saved_errno = errno;
                        break;
                }
                ++ddc->rec_in;
                r = splice(p[0], NULL, ofd, NULL, r, SPLICE_F_MORE);
                if (r <= 0) {
                        ddc->saved_errno = errno;
                        break;
                }
                ddc->b_out += r;
                ++ddc->rec_out;
        }
        close(ifd);
        close(ofd);
        close(p[0]);
        close(p[1]);
        if (r < 0)
                return -1;
        return 0;
}

static int
copy(struct dd_config *ddc)
{
	int r = 0;

	ddc->t_start = time(NULL);

	r = copy_splice(ddc);
	ddc->t_end = time(NULL);

	/* avoid div by zero */
	if (ddc->t_start == ddc->t_end)
		++ddc->t_end;
	return r;
}

static void
usage(void)
{
	eprintf("usage: %s [-h] [if=infile] [of=outfile] [bs[=N]] [seek=N] [skip=N] [count=N] [direct] [quiet] [nosync]\n", argv0);
}

static void
print_stat(const struct dd_config *ddc)
{
	if (ddc->quiet)
		return;

	fprintf(stderr, "%"PRIu64" records in\n", ddc->rec_in);
	fprintf(stderr, "%"PRIu64" records out\n", ddc->rec_out);
	fprintf(stderr, "%"PRIu64" bytes (%"PRIu64" MB) copied", ddc->b_out,
		ddc->b_out/(1<<20));
	fprintf(stderr, ", %lu s, %f MB/s\n",
		(unsigned long)ddc->t_end - ddc->t_start,
		((double)(ddc->b_out/(1<<20)))/(ddc->t_end - ddc->t_start));
}

static void
sig_int(int unused)
{
	(void) unused;
	fprintf(stderr, "SIGINT! Aborting ...\n");
	sigint = 1;
}

int
main(int argc, char *argv[])
{
	int i = 0;
	char buf[1024];
	struct dd_config config;

	argv0 = argv[0];
	memset(&config, 0, sizeof(config));
	config.bs = 1<<16;
	config.in = "/dev/stdin";
	config.out = "/dev/stdout";

	/* emulate 'dd' argument parsing */
	for (i = 1; i < argc; ++i) {
		memset(buf, 0, sizeof(buf));
		if (sscanf(argv[i], "if=%1023s", buf) == 1)
			config.in = strdup(buf);
		else if (sscanf(argv[i], "of=%1023s", buf) == 1)
			config.out = strdup(buf);
		else if (sscanf(argv[i], "skip=%1023s", buf) == 1)
			config.skip = estrtoul(buf, 0);
		else if (sscanf(argv[i], "seek=%1023s", buf) == 1)
			config.seek = estrtoul(buf, 0);
		else if (sscanf(argv[i], "count=%1023s", buf) == 1)
			config.count = estrtoul(buf, 0);
		else if (strcmp(argv[i], "direct") == 0)
			config.direct = 1;
		else if (sscanf(argv[i], "bs=%1023s", buf) == 1)
			config.bs = estrtoul(buf, 0);
		else if (strcmp(argv[i], "bs") == 0)
			config.bs = 0;
		else if (strcmp(argv[i], "quiet") == 0)
			config.quiet = 1;
		else if (strcmp(argv[i], "nosync") == 0)
			config.nosync = 1;
		else if (strcmp(argv[i], "-h") == 0)
			usage();
	}

	if (!config.in || !config.out)
		usage();

	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sig_int);

	if (copy(&config) < 0)
		weprintf("copy:");
	print_stat(&config);

	if (config.nosync == 0)
		sync(); sync();
	return config.saved_errno;
}
