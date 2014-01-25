/* See LICENSE file for copyright and license details. */

struct mntinfo {
	char *fsname;
	char *mntdir;
};

int grabmntinfo(struct mntinfo **minfo);
