/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../util.h"

void
devtotty(int dev, int *tty_maj, int *tty_min)
{
	*tty_maj = (dev >> 8) & 0xfff;
	*tty_min = (dev & 0xff) | ((dev >> 12) & 0xfff00);
}

char *
ttytostr(int tty_maj, int tty_min)
{
	const char *pts = "pts/";
	const char *tty = "tty/";
	char *ttystr;
	size_t len;

	/* Up to 10k ttys */
	len = strlen(pts) + 4 + 1;
	ttystr = malloc(len);
	if (!ttystr)
		eprintf("malloc:");
	switch (tty_maj) {
	case 136:
		snprintf(ttystr, len, "%s%d", pts, tty_min);
		break;
	case 4:
		snprintf(ttystr, len, "%s%d", tty, tty_min);
	default:
		ttystr[0] = '?';
		ttystr[1] = '\0';
		break;
	}
	return ttystr;
}
