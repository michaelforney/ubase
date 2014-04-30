/* See LICENSE file for copyright and license details. */
#include "arg.h"

#define UTF8_POINT(c) (((c) & 0xc0) != 0x80)
#define LEN(x) (sizeof (x) / sizeof *(x))

extern char *argv0;

char *agetcwd(void);
void apathmax(char **, long *);
void devtotty(int, int *, int *);
void enprintf(int, const char *, ...);
void eprintf(const char *, ...);
long estrtol(const char *, int);
#undef explicit_bzero
void explicit_bzero(void *, size_t);
void putword(const char *);
void recurse(const char *, void (*)(const char *));
#undef strlcat
size_t strlcat(char *, const char *, size_t);
#undef strlcpy
size_t strlcpy(char *, const char *, size_t);
char *ttytostr(int, int);
void weprintf(const char *, ...);
