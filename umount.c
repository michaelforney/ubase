#include <stdio.h>
#include "ubase.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-f] target\n", argv0);
}

int
main(int argc, char *argv[]) {
	int i;
	int fflag = 0;
	int ret = 0;

	ARGBEGIN {
	case 'f':
		fflag = UBASE_MNT_FORCE;
		break;
	default:
		usage();
	} ARGEND;
	if (argc < 1)
		usage();
	for (i = 0; i < argc; i++) {
		if (do_umount(argv[i], fflag) < 0)
			eprintf("do_umount:");
		ret = 1;
	}
	return ret;
}
