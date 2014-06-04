# ubase version
VERSION = 0.1

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

#CC = gcc
#CC = musl-gcc
LD = $(CC)
CPPFLAGS = -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS   = -std=c99 -Wall -Wextra $(CPPFLAGS)
LDFLAGS  = -s -lcrypt # -static
