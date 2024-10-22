/*
 * Copyright (C) 2012 Feng Shuo <steve.shuo.feng@gmail.com>
 *
 * This file is part of HSFS.
 *
 * HSFS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * HSFS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HSFS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _HSI_NFS_H_
#define _HSI_NFS_H_

#include <hsfs.h>
#include <hsfs/err.h>

#include <nfs_xdr.h>

/*
 * This is the kernel NFS client file handle representation
 */
#define NFS_MAXFHSIZE 128

struct nfs_fh {
	unsigned short size;
	unsigned char data[NFS_MAXFHSIZE];
};

/*
 * Returns a zero iff the size and data fields match.
 * Checks only "size" bytes in the data field.
 */
static inline int nfs_compare_fh(const struct nfs_fh *a, const struct nfs_fh *b)
{
	return a->size != b->size || memcmp(a->data, b->data, a->size) != 0;
}

static inline void nfs_copy_fh(struct nfs_fh *target, const struct nfs_fh *source)
{
	target->size = source->size;
	memcpy(target->data, source->data, source->size);
}

static inline void nfs_copy_fhi(struct nfs_fh *target, int len, const char *source)
{
	target->size = len;
	memcpy(target->data, source, len);
}

/* The nfs_copy_fh to work with rpc_gen style */
static inline void nfs_copy_fh3(struct nfs_fh *target, int len, const char *source)
{
	target->size = len;
	memcpy(target->data, source, len);
}

/*
 * This is really a general kernel constant, but since nothing like
 * this is defined in the kernel headers, I have to do it here.
 */
#define NFS_OFFSET_MAX ((int64_t)((~(__u64)0) >> 1))


enum nfs3_stable_how {
	NFS_UNSTABLE = 0,
	NFS_DATA_SYNC = 1,
	NFS_FILE_SYNC = 2
};

/* For containerof..... */
#include <hsfs/list.h>

static inline void nfs_init_fattr(struct nfs_fattr *attr)
{
	attr->valid = 0;	/* This seems enough already */
}

struct nfs_inode{
	uint64_t fileid;
	struct nfs_fh fh;
	unsigned long flags;
	unsigned long attrtimeo;
	struct hsfs_inode hsfs_inode;
	uint64_t cookieverf;
};

/*
 * Bit offsets in flags field
 */
#define NFS_INO_ADVISE_RDPLUS (1U << 0) /* advise readdirplus */
#define NFS_INO_STALE (1U << 1)		/* possible stale inode */
#define NFS_INO_ACL_LRU_SET (1U << 2)	/* Inode is on the LRU list */
#define NFS_INO_MOUNTPOINT (1U << 3)	/* inode is remote mountpoint */
#define NFS_INO_FLUSHING (1U << 4)	/* inode is flushing out data */
#define NFS_INO_FSCACHE (1U << 5)	/* inode can be cached by FS-Cache */
#define NFS_INO_FSCACHE_LOCK (1U << 6)	/* Fs-Cache cookie management lock */

static inline struct nfs_inode *NFS_I(const struct hsfs_inode *inode)
{
	return container_of(inode, struct nfs_inode, hsfs_inode);
}

static inline struct nfs_fh *NFS_FH(const struct hsfs_inode *inode)
{
	return &NFS_I(inode)->fh;
}

static inline uint64_t NFS_FILEID(const struct hsfs_inode *inode)
{
	return NFS_I(inode)->fileid;
}

static inline void set_nfs_fileid(struct hsfs_inode *inode, uint64_t fileid)
{
	NFS_I(inode)->fileid = fileid;
	inode->real_ino = fileid;
}

static inline int NFS_STALE(const struct hsfs_inode *inode)
{
	return NFS_INO_STALE & NFS_I(inode)->flags;
}

/* 
 * Be aware that the following two functions are only right with
 * -D_FILE_OFFSET_BITS=64
 */
static inline off_t nfs_size_to_off_t(uint64_t size)
{
	if (size > (__u64) LLONG_MAX - 1)
		return LLONG_MAX - 1;
	return (off_t) size;
}

/*
 * Calculate the number of 512byte blocks used.
 */
static inline blkcnt_t nfs_calc_block_size(uint64_t tsize)
{
	blkcnt_t used = (tsize + 511) >> 9;
	return (used > LLONG_MAX) ? LLONG_MAX : used;
}

struct hsfs_inode *
hsi_nfs_fhget(struct hsfs_super *sb, struct nfs_fh *fh, struct nfs_fattr *fattr);

extern struct hsfs_super_ops hsi_nfs_sop;
extern int nfs_refresh_inode(struct hsfs_inode *inode, struct nfs_fattr *fattr);
extern int hsi_nfs_setattr(struct hsfs_inode *inode, struct hsfs_iattr *attr);
extern void nfs_destroy_inode(struct hsfs_inode *inode);
extern struct hsfs_inode *nfs_alloc_inode(struct hsfs_super *sb);


static inline void
hsfs_nfs_getfhi(struct hsfs_inode *inode, struct nfs_fhi *fh)
{
	fh->len = NFS_FH(inode)->size;
	fh->val = (char *)NFS_FH(inode)->data;
}

void hsfs_log_fattr(struct nfs_fattr *fattr);
void hsfs_log_nfsfh(struct nfs_fh *nfh);
void hsfs_log_super(struct hsfs_super *sb);

#endif	/* _HSI_NFS_H_ */
