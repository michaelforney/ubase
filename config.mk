# ubase version
VERSION = 0.0

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

#CC = gcc
#CC = musl-gcc
LD = $(CC)
CPPFLAGS = -D_BSD_SOURCE -D_GNU_SOURCE
CFLAGS   = -g -std=c99 -Wall -Wextra $(CPPFLAGS)
LDFLAGS  = -g -lcrypt

#CC = tcc
#LD = $(CC)
#CPPFLAGS = -D_BSD_SOURCE -D_GNU_SOURCE
#CFLAGS   = -Os -Wall $(CPPFLAGS)
#LDFLAGS  = -lcrypt
