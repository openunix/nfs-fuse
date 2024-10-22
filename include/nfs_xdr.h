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

/* This file is copied from Linux Kernel include/linux/nfs_xdr.h */
#ifndef _NFS_XDR_H
#define _NFS_XDR_H

#include <stdint.h>
#include <sys/stat.h>

/*
 * To change the maximum rsize and wsize supported by the NFS client, adjust
 * NFS_MAX_FILE_IO_SIZE.  64KB is a typical maximum, but some servers can
 * support a megabyte or more.  The default is left at 4096 bytes, which is
 * reasonable for NFS over UDP.
 */
#define NFS_MAX_FILE_IO_SIZE	(1048576U)
#define NFS_DEF_FILE_IO_SIZE	(4096U)
#define NFS_MIN_FILE_IO_SIZE	(1024U)

struct nfs_fsid {
	uint64_t		major;
	uint64_t		minor;
};

/*
 * Helper for checking equality between 2 fsids.
 */
static inline int nfs_fsid_equal(const struct nfs_fsid *a, const struct nfs_fsid *b)
{
	return a->major == b->major && a->minor == b->minor;
}

struct nfs_fattr {
	unsigned int valid;/* which fields are valid */
	mode_t mode;
	uint32_t nlink;
	uint32_t uid;
	uint32_t gid;
	dev_t rdev;
	uint64_t size;
	union {
		struct {
			uint32_t blocksize;
			uint32_t blocks;
		} nfs2;
		struct {
			uint64_t used;
		} nfs3;
	} du;
	struct nfs_fsid fsid;
	uint64_t fileid;
	struct timespec atime;
	struct timespec mtime;
	struct timespec ctime;
	uint64_t change_attr;/* NFSv4 change attribute */
	uint64_t pre_change_attr;/* pre-op NFSv4 change attribute */
	uint64_t pre_size;/* pre_op_attr.size  */
	struct timespec pre_mtime;/* pre_op_attr.mtime  */
	struct timespec pre_ctime;/* pre_op_attr.ctime  */
	unsigned long time_start;
	unsigned long gencount;
};

#define NFS_ATTR_FATTR_TYPE (1U << 0)
#define NFS_ATTR_FATTR_MODE (1U << 1)
#define NFS_ATTR_FATTR_NLINK (1U << 2)
#define NFS_ATTR_FATTR_OWNER (1U << 3)
#define NFS_ATTR_FATTR_GROUP (1U << 4)
#define NFS_ATTR_FATTR_RDEV (1U << 5)
#define NFS_ATTR_FATTR_SIZE (1U << 6)
#define NFS_ATTR_FATTR_PRESIZE (1U << 7)
#define NFS_ATTR_FATTR_BLOCKS_USED (1U << 8)
#define NFS_ATTR_FATTR_SPACE_USED (1U << 9)
#define NFS_ATTR_FATTR_FSID (1U << 10)
#define NFS_ATTR_FATTR_FILEID (1U << 11)
#define NFS_ATTR_FATTR_ATIME (1U << 12)
#define NFS_ATTR_FATTR_MTIME (1U << 13)
#define NFS_ATTR_FATTR_CTIME (1U << 14)
#define NFS_ATTR_FATTR_PREMTIME (1U << 15)
#define NFS_ATTR_FATTR_PRECTIME (1U << 16)
#define NFS_ATTR_FATTR_CHANGE (1U << 17)
#define NFS_ATTR_FATTR_PRECHANGE (1U << 18)
#define NFS_ATTR_FATTR_V4_REFERRAL (1U << 19)/* NFSv4 referral */

#define NFS_ATTR_FATTR (NFS_ATTR_FATTR_TYPE \
	| NFS_ATTR_FATTR_MODE \
	| NFS_ATTR_FATTR_NLINK \
	| NFS_ATTR_FATTR_OWNER \
	| NFS_ATTR_FATTR_GROUP \
	| NFS_ATTR_FATTR_RDEV \
	| NFS_ATTR_FATTR_SIZE \
	| NFS_ATTR_FATTR_FSID \
	| NFS_ATTR_FATTR_FILEID \
	| NFS_ATTR_FATTR_ATIME \
	| NFS_ATTR_FATTR_MTIME \
			| NFS_ATTR_FATTR_CTIME)
#define NFS_ATTR_FATTR_V2 (NFS_ATTR_FATTR \
			   | NFS_ATTR_FATTR_BLOCKS_USED)
#define NFS_ATTR_FATTR_V3 (NFS_ATTR_FATTR \
			   | NFS_ATTR_FATTR_SPACE_USED)
#define NFS_ATTR_FATTR_V4 (NFS_ATTR_FATTR \
	| NFS_ATTR_FATTR_SPACE_USED \
			   | NFS_ATTR_FATTR_CHANGE)

struct nfs_fhi {
	int len;
	char *val;
};

#endif
