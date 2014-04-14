# ubase version
VERSION = 0.0

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

#CC = gcc
#CC = musl-gcc
LD = $(CC)
CPPFLAGS = -D_GNU_SOURCE
CFLAGS   = -Os -Wall -Wextra $(CPPFLAGS)
LDFLAGS  = -s -lcrypt # -static
