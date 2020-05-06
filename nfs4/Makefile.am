AM_CPPFLAGS = $(TIRPC_HEADERS) -I$(top_srcdir)/include
AM_CFLAGS = -Wall -Wextra

NFS4_X = nfs4.x
HSFS_NFS4_X = nfs4.xdr
HSFS_NFS4_H = $(top_builddir)/include/nfs4.h
HSFS_NFS4_XDR = nfs4xdr.c

noinst_LIBRARIES = libnfs4.a
libnfs4_a_SOURCES = $(HSFS_NFS4_XDR)

EXTRA_DIST = $(NFS4_X)
CLEANFILES = $(HSFS_NFS4_X) $(HSFS_NFS4_XDR) $(HSFS_NFS4_H)

# Copy the xdr file into the build dir, which is different in VPATH build
$(HSFS_NFS4_X) : $(NFS4_X)
	cp -f $< $@

# Generate the headers and xdr codes
$(HSFS_NFS4_H) : $(HSFS_NFS4_X)
	test -f $@ && rm -rf $@ || true
	rpcgen -h -o $@ $<

$(HSFS_NFS4_XDR) : $(HSFS_NFS4_X) $(HSFS_NFS4_H)
	test -f $@ && rm -rf $@ || true
	rpcgen -c -C -o $@ $<
