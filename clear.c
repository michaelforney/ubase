/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	printf("\e[2J\e[H");
	return EXIT_SUCCESS;
}
