/* See LICENSE file for copyright and license details. */
#include "arg.h"

#define UTF8_POINT(c) (((c) & 0xc0) != 0x80)
#define LEN(x) (sizeof (x) / sizeof *(x))

extern char *argv0;

void eprintf(const char *, ...);
void enprintf(int, const char *, ...);
long estrtol(const char *, int);
