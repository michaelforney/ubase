/* See LICENSE file for copyright and license details. */
struct procstat {
	int pid;
	char comm[PATH_MAX + 2]; /* + 2 for '(' and ')' */
	unsigned char state;
	int ppid;
	int pgrp;
	int sid;
	int tty_nr;
	int tpgid;
	unsigned flags;
	unsigned long minflt;
	unsigned long cminflt;
	unsigned long majflt;
	unsigned long cmajflt;
	unsigned long utime;
	unsigned long stime;
	long cutime;
	long cstime;
	long priority;
	long nice;
	long num_threads;
	long itrealvalue;
	unsigned long long starttime;
	unsigned long vsize;
	long rss;
	long rsslim;
};

int parsestat(pid_t pid, struct procstat *ps);
int proceuid(pid_t pid, uid_t *euid);
int validps(const char *path);
