/* See LICENSE file for copyright and license details. */

/* grabmntinfo.c */
struct mntinfo {
	const char *fsname;
	const char *mntdir;
};

int grabmntinfo(struct mntinfo **minfo);

/* syslog.c */
int syslog_size(void);
int syslog_read(void *buf, size_t n);
