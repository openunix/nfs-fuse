AM_CPPFLAGS = $(FUSE_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)
AM_CFLAGS = -Wall -Wextra

noinst_LIBRARIES = libhsfs.a libnfsi.a
libhsfs_a_CPPFLAGS = $(AM_CPPFLAGS) $(TIRPC_HEADERS)
libhsfs_a_SOURCES = hsfs_inode.c hsfs_log.c

libnfsi_a_CPPFLAGS = $(AM_CPPFLAGS) -I/usr/include/nfsc \
		     "-D_U_=__attribute__((unused))"
libnfsi_a_SOURCES = nfs_v4.c
