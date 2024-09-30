#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <mntent.h>
#include <pwd.h>
#include <libgen.h>
#include <getopt.h>
#include <errno.h>

#include "log.h"
#include "hsfs.h"
#include "hsx_fuse.h"
#include "xcommon.h"
#include "mount_constants.h"
#include "fstab.h"
#include "nfs_mntent.h"

int __INIT_DEBUG = 0;

char *progname = NULL;
int verbose = 0;
int nomtab = 0;
int fg = 0;

static struct option hsfs_opts[] = {
	{ "foreground", 0, 0, 'f' },
	{ "help", 0, 0, 'h' },
	{ "no-mtab", 0, 0, 'n' },
	{ "read-only", 0, 0, 'r' },
	{ "ro", 0, 0, 'r' },
	{ "read-write", 0, 0, 'w' },
	{ "rw", 0, 0, 'w' },
	{ "verbose", 0, 0, 'v' },
	{ "version", 0, 0, 'V' },
	{ "options", 1, 0, 'o' },
	{ NULL, 0, 0, 0 }
};

static void exit_usage(int err)
{
	printf(
"usage: %s remotetarget dir [-rvVwfnh] [-o nfsoptions]\n",
	program_invocation_short_name);
	if (err)
		exit(1);

	printf("options:\n\t-r\t\tMount file system readonly\n");
	printf("\t-v\t\tVerbose\n");
	printf("\t-w\t\tMount file system read-write\n");
	printf("\t-n\t\tDo not update /etc/mtab\n");

	fuse_cmdline_help();
	fuse_lowlevel_help();
	printf("    -o nfsoptions\t   Refer mount.nfs-fuse(8) or nfs-fuse(5)\n\n");
	exit(0);
}

static void print_version(void)
{
	printf("%s: %s\n", program_invocation_short_name, PACKAGE_STRING);
	printf("FUSE library version %s\n", fuse_pkgversion());
	fuse_lowlevel_version();
	exit(0);
}

static inline int hsi_fuse_add_opt(struct fuse_args *args, const char *opt)
{
	char **opts = NULL;

	if (args == NULL)
		return EINVAL;
	
	opts = args->argv + (args->argc -1);
	
	return fuse_opt_add_opt(opts, opt);
}

static struct fuse_lowlevel_ops hsfs_oper = {
	.init = hsx_fuse_init,
	.getattr = hsx_fuse_getattr,
	.statfs = hsx_fuse_statfs,
	.lookup = hsx_fuse_lookup,
	.mkdir = hsx_fuse_mkdir,
	.open = hsx_fuse_open,
	.release = hsx_fuse_release,
	.read = hsx_fuse_read,
	.write = hsx_fuse_write,
	.setattr = hsx_fuse_setattr,
	.forget = hsx_fuse_forget,
	.rmdir = hsx_fuse_rmdir,
	.unlink = hsx_fuse_unlink,
	.readlink = hsx_fuse_readlink,
	.symlink = hsx_fuse_symlink,
	.rename = hsx_fuse_rename,
	.readdir = hsx_fuse_readdir,
	.opendir = hsx_fuse_opendir,
	.mknod = hsx_fuse_mknod,
	.link = hsx_fuse_link,
	.create = hsx_fuse_create,
	.access = hsx_fuse_access,
	.getxattr = hsx_fuse_getxattr,
	.setxattr = hsx_fuse_setxattr,
	.readdirplus = hsx_fuse_readdir_plus,
};

/*
 * Map from -o and fstab option strings to the flag argument to mount(2).
 */
struct opt_map {
	const char *nfs_opt;	/* option name */
	int skip;		/* skip in mtab option string */
	int inv;		/* true if flag value should be inverted */
	int mask;		/* flag mask value */
	const char *fuse_opt;	/* for fuse args mapping */
};

/* Custom mount options for our own purposes.  */
/* Maybe these should now be freed for kernel use again */
#define MS_DUMMY	0x00000000
#define MS_USERS	0x40000000
#define MS_USER		0x20000000

static const struct opt_map opt_map[] = {
  { "defaults", 0, 0, 0              , NULL      },    /* default options */
  { "ro",       1, 0, MS_RDONLY      , "ro"      },    /* read-only */
  { "rw",       1, 1, MS_RDONLY      , "rw"      },    /* read-write */
  { "exec",     0, 1, MS_NOEXEC      , "exec"    },    /* permit execution of binaries */
  { "noexec",   0, 0, MS_NOEXEC      , "noexec"  },    /* don't execute binaries */
  { "suid",     0, 1, MS_NOSUID      , "suid"    },    /* honor suid executables */
  { "nosuid",   0, 0, MS_NOSUID      , "nosuid"  },    /* don't honor suid executables */
  { "dev",      0, 1, MS_NODEV       , "dev"     },    /* interpret device files  */
  { "nodev",    0, 0, MS_NODEV       , "nodev"   },    /* don't interpret devices */
  { "sync",     0, 0, MS_SYNCHRONOUS , "sync"    },    /* synchronous I/O */
  { "async",    0, 1, MS_SYNCHRONOUS , "async"   },    /* asynchronous I/O */
  { "dirsync",  0, 0, MS_DIRSYNC     , "dirsync" },    /* synchronous directory modifications */
  { "remount",  0, 0, MS_REMOUNT     , NULL      },    /* Alter flags of mounted FS */
  { "bind",     0, 0, MS_BIND        , NULL      },    /* Remount part of tree elsewhere */
  { "rbind",    0, 0, MS_BIND|MS_REC , NULL      },    /* Idem, plus mounted subtrees */
  { "auto",     0, 0, MS_DUMMY       , NULL      },    /* Can be mounted using -a */
  { "noauto",   0, 0, MS_DUMMY       , NULL      },    /* Can  only be mounted explicitly */
  { "users",    0, 0, MS_USERS       , "allow_other" },/* Allow ordinary user to mount */
  { "nousers",  0, 1, MS_USERS       , "allow_root"  },/* Forbid ordinary user to mount */
  { "user",     0, 0, MS_USER        , "allow_other" },/* Allow ordinary user to mount */
  { "nouser",   0, 1, MS_USER        , "allow_root"  },/* Forbid ordinary user to mount */
  { "owner",    0, 0, MS_DUMMY       , NULL      },    /* Let the owner of the device mount */
  { "noowner",  0, 0, MS_DUMMY       , NULL      },    /* Device owner has no special privs */
  { "group",    0, 0, MS_DUMMY       , NULL      },    /* Let the group of the device mount */
  { "nogroup",  0, 0, MS_DUMMY       , NULL      },    /* Device group has no special privs */
  { "_netdev",  0, 0, MS_DUMMY       , NULL      },    /* Device requires network */
  { "comment",  0, 0, MS_DUMMY       , NULL      },    /* fstab comment only (kudzu,_netdev)*/

  /* add new options here */
#ifdef MS_NOSUB
  { "sub",      0, 1, MS_NOSUB       , NULL      },    /* allow submounts */
  { "nosub",    0, 0, MS_NOSUB       , NULL      },    /* don't allow submounts */
#endif
#ifdef MS_SILENT
  { "quiet",    0, 0, MS_SILENT      , NULL      },    /* be quiet  */
  { "loud",     0, 1, MS_SILENT      , NULL      },    /* print out messages. */
#endif
#ifdef MS_MANDLOCK
  { "mand",     0, 0, MS_MANDLOCK    , NULL      },    /* Allow mandatory locks on this FS */
  { "nomand",   0, 1, MS_MANDLOCK    , NULL      },    /* Forbid mandatory locks on this FS */
#endif
  { "loop",     1, 0, MS_DUMMY       , NULL      },    /* use a loop device */
#ifdef MS_NOATIME
  { "atime",    0, 1, MS_NOATIME     , "atime"   },     /* Update access time */
  { "noatime",  0, 0, MS_NOATIME     , "noatime" },     /* Do not update access time */
#endif
#ifdef MS_NODIRATIME
  { "diratime", 0, 1, MS_NODIRATIME  , NULL      },  /* Update dir access times */
  { "nodiratime", 0, 0, MS_NODIRATIME, NULL      },/* Do not update dir access times */
#endif
  { NULL,	0, 0, 0, NULL	}
};

/* Try to build a canonical options string.  */
static char * hsi_fix_opts_string (int flags, const char *extra_opts) {
	const struct opt_map *om;
	char *new_opts;

	new_opts = xstrdup((flags & MS_RDONLY) ? "ro" : "rw");
	if (flags & MS_USER) {
		struct passwd *pw = getpwuid(getuid());
		if(pw)
			new_opts = xstrconcat3(new_opts, ",user=", pw->pw_name);
	}
	
	for (om = opt_map; om->nfs_opt != NULL; om++) {
		if (om->skip)
			continue;
		if (om->inv || !om->mask || (flags & om->mask) != om->mask)
			continue;
		new_opts = xstrconcat3(new_opts, ",", om->nfs_opt);
		flags &= ~om->mask;
	}
	if (extra_opts && *extra_opts) {
		new_opts = xstrconcat3(new_opts, ",", extra_opts);
	}

	return new_opts;
}

static inline void dup_mntent(struct mntent *ment, nfs_mntent_t *nment)
{
	/* Not sure why nfs_mntent_t should exist */
	nment->mnt_fsname = strdup(ment->mnt_fsname);
	nment->mnt_dir = strdup(ment->mnt_dir);
	nment->mnt_type = strdup(ment->mnt_type);
	nment->mnt_opts = strdup(ment->mnt_opts);
	nment->mnt_freq = ment->mnt_freq;
	nment->mnt_passno = ment->mnt_passno;
}
static inline void free_mntent(nfs_mntent_t *ment, int remount)
{
	free(ment->mnt_fsname);
	free(ment->mnt_dir);
	free(ment->mnt_type);
	/* 
	 * Note: free(ment->mnt_opts) happens in discard_mntentchn()
	 * via update_mtab() on remouts
	 */
	 if (!remount)
	 	free(ment->mnt_opts);
}

static int hsi_add_mtab(const char *spec, const char *mount_point,
				char *fstype, int flags, char *opts)
{
	struct mntent ment;
	FILE *mtab;
	int res = 1;

	if (nomtab)
		return 0;

	ment.mnt_fsname = (char *)spec;
	ment.mnt_dir = (char *)mount_point;
	ment.mnt_type = fstype;
	ment.mnt_opts = hsi_fix_opts_string(flags, opts);
	ment.mnt_freq = 0;
	ment.mnt_passno= 0;

	if(flags & MS_REMOUNT) {
		nfs_mntent_t nment;
		
		dup_mntent(&ment, &nment);
		update_mtab(nment.mnt_dir, &nment);
		free_mntent(&nment, 1);
		return 0;
	}

	lock_mtab();

	if ((mtab = setmntent(MOUNTED, "a+")) == NULL) {
		ERR("Can't open " MOUNTED);
		goto end;
	}

	if (addmntent(mtab, &ment) == 1) {
		ERR("Can't write mount entry");
		goto end;
	}

	endmntent(mtab);
	res = 0;
end:
	unlock_mtab();
	return res;
}

static int hsi_del_mtab(const char *node)
{
	if (nomtab)
		return 0;

	update_mtab (node, NULL);

	return 0;
}

static void hsi_parse_opt(const char *opt, int *flags, struct fuse_args *args,
					char *extra_opts, size_t len)
{
	const struct opt_map *om = NULL;

	for (om = opt_map; om->nfs_opt != NULL; om++) {
		if (!strcmp (opt, om->nfs_opt)) {
			if (om->fuse_opt) {
				hsi_fuse_add_opt(args, om->fuse_opt);
				if (om->inv)
					*flags &= ~om->mask;
				else
					*flags |= om->mask;
			} else {
				WARNING("Not supported opt: %s.", opt);
			}
			return;
		} else if (fuse_lowlevel_is_lib_option(opt)) {
			hsi_fuse_add_opt(args, opt);
			return;
		}
	}

	len -= strlen(extra_opts);

	if (*extra_opts && --len > 0)
		strcat(extra_opts, ",");

	if ((len -= strlen(opt)) > 0)
		strcat(extra_opts, opt);
}


static void hsi_parse_opts(const char *options, int *flags, 
				struct fuse_args *args, char **udata)
{
	if (options != NULL) {
		char *opts = xstrdup(options);
		char *opt = NULL, *p = NULL;
		size_t len = strlen(opts) + 1;	/* include room for a null */
		int open_quote = 0;

		*udata = xmalloc(len);
		**udata = '\0';

		for (p = opts, opt = NULL; p && *p; p++) {
			if (!opt)
				opt = p;	/* begin of the option item */
			if (*p == '"')
				open_quote ^= 1; /* reverse the status */
			if (open_quote)
				continue;	/* still in a quoted block */
			if (*p == ',')
				*p = '\0';	/* terminate the option item */

			/* end of option item or last item */
			if (*p == '\0' || *(p + 1) == '\0') {
				hsi_parse_opt(opt, flags, args, *udata, len);
				opt = NULL;
			}
		}
		free(opts);
	}
}

#define HSFS_CMD_OPT(t, p) \
	{ t, offsetof(struct hsfs_cmdline_opts, p), 1 }
static const struct fuse_opt hsfs_cmdline_spec[] = {
	/* CMD options, -r, -V, -w, -f, -h will be handled by fuse,
	while -f has a different meaning in fuse, use --fake instead. */
	HSFS_CMD_OPT("--fake", fake),
	FUSE_OPT_KEY("-v", 'v'),
	FUSE_OPT_KEY("-f", 'f'),
	FUSE_OPT_END
};

static int hsfs_cmdline_proc(void *data, const char *arg, int key,
			_U_ struct fuse_args *outargs)
 {
	struct hsfs_cmdline_opts *hsfs_opts = data;
	int c = 0, ret = 0;

	c = key;
		switch(c) {
		case 'r':
			hsfs_opts->flags |= MS_RDONLY;
			break;
		case 'v':
			++verbose;
			break;
		case 'w':
			hsfs_opts->flags &= ~MS_RDONLY;
			break;
		case 'f':
			hsfs_opts->fg = 1;
			ret = 1;
			break;
		case 'n':
			nomtab = 1;
		case FUSE_OPT_KEY_NONOPT:
			if (hsfs_opts->spec)
				ret = 1;	/* mount point */
			else
				hsfs_opts->spec = xstrdup(arg);
			break;
		default:
			ret = fuse_opt_add_opt(&(hsfs_opts->udata), arg);
			if (ret == 0)
				ret = 1; /* currently we send everything to fuse as we don't know what they want. */
			break;
		}
	return ret;
}

static int hsfs_parse_cmdline(struct fuse_args *args,
		struct fuse_cmdline_opts *fuse_opts,
		struct hsfs_cmdline_opts *hsfs_opts)
{
	int ret;

	ret = fuse_opt_parse(args, hsfs_opts, hsfs_cmdline_spec, hsfs_cmdline_proc);
	if (ret != 0)
		goto out_usage;

	/* add subtype args */
	ret = fuse_parse_cmdline(args, fuse_opts);
	if (ret != 0)
		goto out_usage;
	if (fuse_opts->show_help)
		goto out_usage;
	if (fuse_opts->show_version) {
		print_version();
			goto out;
	}

	/* add fs mode */
	if (hsfs_opts->spec == NULL){
		ret = 1;
		goto out_usage;
	}
	if (fuse_opts->mountpoint == NULL) {
		ret = 1;
		goto out_usage;
	}

	return 0;

out_usage:
	exit_usage(ret);
	/* Never reach here... */
out:
	return ret;
}

int main(int argc, char **argv)
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_cmdline_opts fuse_opts;
	struct hsfs_cmdline_opts hsfs_opts;
	struct fuse_session *se = NULL;
	struct hsfs_super super;
	int err = -1;

	bzero(&hsfs_opts, sizeof(hsfs_opts));
	bzero(&super, sizeof(struct hsfs_super));
	progname = program_invocation_short_name;

	/* It actually never returns errors. */
	err = hsfs_parse_cmdline(&args, &fuse_opts, &hsfs_opts);
	if (err != 0)
		goto out;

	err = hsfs_init();
	if (err)
		goto out;

	err = hsfs_do_mount(&hsfs_opts, &super);
	if (err)
		goto out;

	err = -1;
	se = fuse_session_new(&args, &hsfs_oper, sizeof(hsfs_oper), &super);
	if (se == NULL)
		goto err_out1;
	if (fuse_set_signal_handlers(se) != 0)
	    goto err_out2;
	if (fuse_session_mount(se, fuse_opts.mountpoint) != 0)
	    goto err_out3;
	fuse_daemonize(fuse_opts.foreground);

	if (fuse_opts.singlethread)
			err = fuse_session_loop(se);
	else {
		struct fuse_loop_config config = {
			.clone_fd = fuse_opts.clone_fd,
			.max_idle_threads = fuse_opts.max_idle_threads,
		};

		err = fuse_session_loop_mt(se, &config);
	}
	fuse_session_unmount(se);
err_out3:
			fuse_remove_signal_handlers(se);
err_out2:
		fuse_session_destroy(se);
err_out1:
	hsfs_do_unmount(&hsfs_opts, &super);
out:
	if (hsfs_opts.udata)
		free(hsfs_opts.udata);
	if (hsfs_opts.spec)
		free(hsfs_opts.spec);
	if (fuse_opts.mountpoint)
		free(fuse_opts.mountpoint);

	fuse_opt_free_args(&args);

	return err ? 1 : 0;
}
