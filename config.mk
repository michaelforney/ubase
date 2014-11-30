# ubase version
VERSION = 0.1

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = cc
LD = $(CC)
AR = ar
RANLIB = ranlib

CPPFLAGS = -D_XOPEN_SOURCE -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS   = -std=c99 -Wall -Wextra
LDLIBS   = -lcrypt
LDFLAGS  = -s
