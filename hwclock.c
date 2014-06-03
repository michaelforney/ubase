/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "rtc.h"
#include "util.h"

static void echotime(char *);
static void readrtctm(struct tm *, int);
static void writertctm(struct tm *, int);
static void writetime(char *);

static void
usage(void)
{
	eprintf("usage: %s [-rw] [-u]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *dev = "/dev/rtc";
	int rflag = 0;
	int wflag = 0;

	ARGBEGIN {
	case 'r':
		rflag = 1;
		break;
	case 'w':
		wflag = 1;
		break;
	case 'u':
		break;
	default:
		usage();
	} ARGEND;

	if ((rflag ^ wflag) == 0)
		eprintf("missing or incompatible function\n");

	/* Only UTC support at the moment */
	setenv("TZ", "UTC0", 1);
	tzset();

	if (rflag == 1)
		echotime(dev);
	else if (wflag == 1)
		writetime(dev);

	return EXIT_SUCCESS;
}

static void
echotime(char *dev)
{
	struct tm tm;
	time_t t;
	int fd;

	fd = open(dev, O_RDONLY);
	if (fd < 0)
		eprintf("open %s:", dev);
	readrtctm(&tm, fd);
	t = mktime(&tm);
	printf("%s", asctime(localtime(&t)));
	close(fd);
}

static void
readrtctm(struct tm *tm, int fd)
{
	memset(tm, 0, sizeof(*tm));
	ioctl(fd, RTC_RD_TIME, tm);
}

static void
writertctm(struct tm *tm, int fd)
{
	ioctl(fd, RTC_SET_TIME, tm);
}

static void
writetime(char *dev)
{
	struct timeval tv;
	struct tm *tm;
	time_t t;
	int fd;

	fd = open(dev, O_WRONLY);
	if (fd < 0)
		eprintf("open %s:", dev);
	gettimeofday(&tv, NULL);
	t = tv.tv_sec;
	tm = gmtime(&t);
	writertctm(tm, fd);
	close(fd);
}
