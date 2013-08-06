#include <sys/klog.h>
#include <stdio.h>

enum {
	SYSLOG_ACTION_READ_ALL = 3,
	SYSLOG_ACTION_SIZE_BUFFER = 10
};

int
syslog_size(void)
{
	return klogctl(SYSLOG_ACTION_SIZE_BUFFER, NULL, 0);
}

int
syslog_read(void *buf, size_t n)
{
	return klogctl(SYSLOG_ACTION_READ_ALL, buf, n);
}
