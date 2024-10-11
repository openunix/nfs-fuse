# This is the must have feature that fuse needed.
AM_CPPFLAGS = $(FUSE_CFLAGS) $(TIRPC_HEADERS) -I$(top_srcdir)/include
AM_CFLAGS = -Wall -Wextra

noinst_LIBRARIES = libmount.a
libmount_a_SOURCES = mount/parse_dev.c mount/parse_dev.h
