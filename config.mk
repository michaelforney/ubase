# ubase version
VERSION = 0.1

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

#CC = gcc
#CC = musl-gcc
LD = $(CC)
CPPFLAGS = -D_GNU_SOURCE
CFLAGS   = -std=c99 -Wall -Wextra -pedantic $(CPPFLAGS)
LDFLAGS  = -s -lcrypt # -static
