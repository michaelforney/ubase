/* See LICENSE file for copyright and license details. */
struct mntinfo {
	const char *fsname;
	const char *mntdir;
};

int grabmntinfo(struct mntinfo **minfo);
