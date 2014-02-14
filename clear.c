/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	printf("\x1b[2J\x1b[H");
	return EXIT_SUCCESS;
}
