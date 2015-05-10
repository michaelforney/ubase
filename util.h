/* See LICENSE file for copyright and license details. */
#include "arg.h"

#define UTF8_POINT(c) (((c) & 0xc0) != 0x80)
#define LEN(x) (sizeof (x) / sizeof *(x))

/* eprintf.c */
extern char *argv0;

/* agetcwd.c */
char *agetcwd(void);

/* apathmax.c */
void apathmax(char **, long *);

/* ealloc.c */
void *ecalloc(size_t, size_t);
void *emalloc(size_t size);
void *erealloc(void *, size_t);
char *estrdup(const char *);

/* eprintf.c */
void enprintf(int, const char *, ...);
void eprintf(const char *, ...);
void weprintf(const char *, ...);

/* estrtol.c */
long estrtol(const char *, int);

/* estrtoul.c */
unsigned long estrtoul(const char *, int);

/* explicit_bzero.c */
#undef explicit_bzero
void explicit_bzero(void *, size_t);

/* putword.c */
void putword(const char *);

/* recurse.c */
void recurse(const char *, void (*)(const char *));

/* strlcat.c */
#undef strlcat
size_t strlcat(char *, const char *, size_t);
size_t estrlcat(char *, const char *, size_t);

/* strlcpy.c */
#undef strlcpy
size_t strlcpy(char *, const char *, size_t);
size_t estrlcpy(char *, const char *, size_t);

/* tty.c */
void devtotty(int, int *, int *);
int ttytostr(int, int, char *, size_t);
