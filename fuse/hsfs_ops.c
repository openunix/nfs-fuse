/*
 * This file is part of nfs-fuse, the FUSE implementation of NFS Client.
 * Copyright (C) 2024 by Feng Shuo <steve.shuo.feng@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fuse_log.h>
#include <hsfs_nfs.h>

#define LOG_FATTR_TIME(f, t) fuse_log(FUSE_LOG_DEBUG, "nfs_fattr: " #t "time: %lld.%ld\n", f->t##time.tv_sec, f->t##time.tv_nsec)

void hsfs_log_fattr(struct nfs_fattr *fattr)
{
	fuse_log(FUSE_LOG_DEBUG, "nfs_fattr: %p, valid: 0x%x\n", fattr, fattr->valid);
	fuse_log(FUSE_LOG_DEBUG, "nfs_fattr: size: %lld, used: %lld\n", fattr->size, fattr->du.nfs3.used);
	fuse_log(FUSE_LOG_DEBUG, "nfs_fattr: nlink: %d, mode: 0x%x\n", fattr->nlink, fattr->mode);
	fuse_log(FUSE_LOG_DEBUG, "nfs_fattr: uid: %d, gid: 0x%d\n", fattr->uid, fattr->gid);
	LOG_FATTR_TIME(fattr, a);
	LOG_FATTR_TIME(fattr, m);
	LOG_FATTR_TIME(fattr, c);

}

void hsfs_log_nfsfh(struct nfs_fh *nfh)
{
	fuse_log(FUSE_LOG_DEBUG, "nfs_fh: %p, size: %d, data: %p\n", nfh, nfh->size, nfh->data);
	fuse_log(FUSE_LOG_DEBUG, "nfs_fh->data: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		 nfh->data[0], nfh->data[1], nfh->data[2], nfh->data[3],
		 nfh->data[4], nfh->data[5], nfh->data[6], nfh->data[7],
		 nfh->data[8], nfh->data[9], nfh->data[10], nfh->data[11],
		 nfh->data[12], nfh->data[13], nfh->data[14], nfh->data[15]);
}

void hsfs_log_super(struct hsfs_super *sb)
{
	fuse_log(FUSE_LOG_DEBUG, "SB: %p\n", sb);
}

#if 0
#include "libnfs.h"
#include "hsfs.h"

int hsfs_do_mount(struct hsfs_cmdline_opts *hsfs_opts,
		  struct hsfs_super *sb)
{
	struct nfs_context *nfs;
	int err = -1;

	if (hsfs_opts->nfsvers != 4)
		return nfs3_do_mount(hsfs_opts, sb);

	nfs = nfs_init_context();
	if (nfs == NULL) {
		err = errno;
		fprintf(stderr, "Failed to init libnfs context\n");
		goto out;
	}

	err = nfs_set_version(nfs, 4);
	if (err) {
		goto out;
	}

	err = try_mount(nfs, hsfs_opts->hostname, hsfs_opts->hostpath);

	printf("NFSv4 is not supported yet. nfs_mount return %d\n", err);

	exit(0);

out:
	return err;
}

int rpc_nfs4_compound_async2(struct rpc_context *rpc, rpc_cb cb,
                                   struct COMPOUND4args *args,
                                   void *private_data,
                                   size_t alloc_hint)
{
	assert(!"Not implemented");
	return -1;
}
#endif