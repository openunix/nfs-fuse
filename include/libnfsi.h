/* This is the interface of our internal libnfs library */
#ifndef _LIBNFSI_H_
#define _LIBNFSI_H_

/* For the cross NFS version data types */
#include "nfs_xdr.h"

#if defined(WIN32) && defined(libnfs_EXPORTS)
#define EXTERN __declspec( dllexport )
#else
#ifndef EXTERN
#define EXTERN
#endif
#endif

/* Same as nfs_mount but returning the post fattr */
EXTERN int nfs_mount2(struct nfs_context *nfs, const char *server,
                      const char *exportname, struct nfs_fattr *fattr);

EXTERN struct nfs_fhi *nfs_get_rootfhi(struct nfs_context *nfs);
#endif