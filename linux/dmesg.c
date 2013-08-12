/* See LICENSE file for copyright and license details. */
#include <sys/klog.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

enum {
	SYSLOG_ACTION_READ_ALL = 3,
	SYSLOG_ACTION_SIZE_BUFFER = 10
};

int
dmesg_size(void)
{
	return klogctl(SYSLOG_ACTION_SIZE_BUFFER, NULL, 0);
}

int
dmesg_read(void *buf, size_t n)
{
	return klogctl(SYSLOG_ACTION_READ_ALL, buf, n);
}

int
dmesg_show(int fd, const void *buf, size_t n)
{
	int last = '\n';
	char newbuf[n], *q = newbuf;
	const char *p = buf;
	size_t i;

	memset(newbuf, 0, n);
	for (i = 0; i < n; ) {
		if (last == '\n' && p[i] == '<') {
			i += 2;
			if (i + 1 < n && p[i + 1] == '>')
				i++;
		} else {
			*q++ = p[i];
		}
		last = p[i++];
	}
	if (write(fd, newbuf, n) != n)
		return -1;
	if (last != '\n')
		if (write(fd, "\n", 1) != 1)
			return -1;
	return 0;
}
