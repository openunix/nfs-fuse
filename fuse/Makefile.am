include $(top_srcdir)/Makefile.common

bin_PROGRAMS = nfs-fuse
nfs_fuse_LDADD = $(top_builddir)/fuse/libhsx_fuse.a $(top_builddir)/nfs3/libhsi_nfs3.a \
	$(top_builddir)/lib/libhsfs.a $(top_builddir)/nfs_common/libhsi_nfsc.a \
	$(top_builddir)/mount/libmount.a $(top_builddir)/nfs4/libnfs4.a

nfs_fuse_SOURCES = xcommon.c main.c

noinst_LIBRARIES = libhsx_fuse.a
libhsx_fuse_a_SOURCES = hsx_fuse_getattr.c hsx_fuse_statfs.c hsx_fuse_init.c \
			hsx_fuse_lookup.c hsx_fuse_fill_reply.c hsx_fuse_mkdir.c \
			hsx_fuse_read.c hsx_fuse_write.c hsx_fuse_open.c hsx_fuse_release.c \
			hsx_fuse_rmdir.c hsx_fuse_forget.c hsx_fuse_setattr.c \
			hsx_fuse_readlink.c hsx_fuse_symlink.c \
			hsx_fuse_unlink.c hsx_fuse_rename.c hsx_fuse_readdir.c \
			hsx_fuse_opendir.c hsx_fuse_setxattr.c \
			hsx_fuse_mknod.c hsx_fuse_link.c hsx_fuse_create.c \
			hsx_fuse_access.c hsx_fuse_getxattr.c hsx_fuse_stat2iattr.c \
            fuse_misc.h

