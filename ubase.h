/* See LICENSE file for copyright and license details. */

/* grabmntinfo.c */
struct mntinfo {
	const char *fsname;
	const char *mntdir;
};

int grabmntinfo(struct mntinfo **minfo);

/* dmesg.c */
int dmesg_size(void);
int dmesg_read(void *buf, size_t n);
int dmesg_show(int fd, const void *buf, size_t n);

/* umount.c */
enum {
	UBASE_MNT_FORCE = 1 << 0,
	UBASE_MNT_DETACH = 1 << 1
};

int do_umount(const char *target, int opts);
