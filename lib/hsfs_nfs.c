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

/*
 * This contains all the hsfs_ functions calling libnfs.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>

#include "libnfs.h"
#include "hsfs.h"

#include "xcommon.h"
#include "nls.h"

#include "hsfs_nfs.h"
#include "libnfsi.h"

/* TODO: Should some part of codes be in hsfs_main.c? This should equal
 to nfs4_get_sb()/nfs_get_sb() in Linux 2.6 */
int hsfs_do_mount(struct hsfs_cmdline_opts *hsfs_opts,
		  struct hsfs_super *sb)
{
	struct nfs_context *nfs;
	struct nfs_fattr fattr;
	struct nfs_fhi *root_fh;
	struct nfs_fh nfh;
	struct hsfs_inode *root;
	int err = EX_SYSERR;

	/* Temporary short cut for NFSv3 */
	sb->version = hsfs_opts->nfsvers;
	if (hsfs_opts->nfsvers != 4)
		return nfs3_do_mount(hsfs_opts, sb);

	nfs = nfs_init_context();
	if (nfs == NULL) {
		nfs_error(_("%s: Failed to init libnfs: %d"), progname, errno);
		goto out;
	}
	if (hsfs_opts->nfsvers) {
		err = nfs_set_version(nfs, 4);
		if (err) {
			err = EX_FAIL;
			goto out2;
		}
	}
	if ((hsfs_opts->debug & HSFS_DEBUG_LIBNFS))
		nfs_set_debug(nfs, hsfs_opts->debug & HSFS_DEBUG_LIBNFS);

	/* TODO: Should be a loop for some times and then go background if needed */
	err = nfs_mount2(nfs, hsfs_opts->hostname, hsfs_opts->hostpath, &fattr);
	if (err) {
		err = err < 0 ? EX_SYSERR : EX_FAIL;
		goto out2;
	}

	if ((hsfs_opts->debug & HSFS_DEBUG_OPMOUNT))
		hsfs_log_fattr(&fattr);

	err = hsfs_init_icache(sb);
	if (err) {
		err = EX_SYSERR;
		goto out1;
	}
	root_fh = nfs_get_rootfhi(nfs);

	nfs_copy_fhi(&nfh, root_fh->len, root_fh->val);
	if ((hsfs_opts->debug & HSFS_DEBUG_OPMOUNT))
		hsfs_log_nfsfh(&nfh);


	root = hsi_nfs_fhget(sb, &nfh, &fattr);
	if (IS_ERR(root)) {
		int ret = PTR_ERR(root);
		nfs_error(_("%s: Failed to initialize: %d"), progname, err);
		goto out1;
	}

	if ((hsfs_opts->debug & HSFS_DEBUG_OPMOUNT))
		hsfs_log_super(&root_fh);

	sb->root = root;
	sb->private = nfs;


	return 0;
out2:
	nfs_error(_("%s: %s"), progname, nfs_get_error(nfs));
out1:
	nfs_destroy_context(nfs);
out:
	return err;
}

int hsfs_do_unmount(struct hsfs_cmdline_opts *hsfs_opts,
		    struct hsfs_super *sb)
{
	if (hsfs_opts->nfsvers != 4)
		return nfs3_do_unmount(hsfs_opts, sb);
	/* No unmount for NFS V4 so far */
	return 0;
}