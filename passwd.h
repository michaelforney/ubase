/* See LICENSE file for copyright and license details. */
/* passwd.c */
int pw_check(struct passwd *, const char *);
int pw_copy(int, int, const struct passwd *);
int pw_scan(char *, struct passwd *);
