AM_CPPFLAGS = $(FUSE_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir) -I/usr/include/nfsc
AM_CFLAGS = -Wall -Wextra

noinst_LIBRARIES = libhsfs.a libnfsi.a
libhsfs_a_CPPFLAGS = $(AM_CPPFLAGS) $(TIRPC_HEADERS)
libhsfs_a_SOURCES = hsfs_inode.c

libnfsi_a_CPPFLAGS = $(AM_CPPFLAGS) \
		     "-D_U_=__attribute__((unused))"
libnfsi_a_SOURCES = nfs_v4.c init.c libnfs.c
