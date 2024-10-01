AM_CPPFLAGS = $(FUSE_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir) $(TIRPC_HEADERS)

AM_CFLAGS = -Wall -Wextra

noinst_LIBRARIES = libhsfs.a
libhsfs_a_CPPFLAGS = $(AM_CPPFLAGS)
libhsfs_a_SOURCES = hsfs_inode.c
