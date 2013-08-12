/* See LICENSE file for copyright and license details. */
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/msgbuf.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
syslog_size(void)
{
	int mib[2], msgbufsize;
	size_t len;
	int ret;

	mib[0] = CTL_KERN;
	mib[1] = KERN_MSGBUFSIZE;
	len = sizeof(msgbufsize);
	ret = sysctl(mib, 2, &msgbufsize, &len, NULL, 0);
	if (ret < 0)
		return ret;
	msgbufsize += sizeof(struct msgbuf) - 1;
	return msgbufsize;
}

int
syslog_read(void *buf, size_t n)
{
	int mib[2];
	int ret;

	memset(buf, 0, n);
	mib[0] = CTL_KERN;
	mib[1] = KERN_MSGBUF;
	ret = sysctl(mib, 2, buf, &n, NULL, 0);
	if (ret < 0)
		return ret;
	memmove(buf, ((struct msgbuf *)buf)->msg_bufc, n);
	return n;
}

int
syslog_show(int fd, const void *buf, size_t n)
{
	return write(fd, buf, n);
}
