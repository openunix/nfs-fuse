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

static void exit_usage(int err)
{
	printf(
"usage: %s remotetarget dir [-rvVwfnh] [-t version]\n",
	program_invocation_short_name);
	if (err)
		exit(1);

	fuse_cmdline_help();
	fuse_lowlevel_help();
	exit(0);
}

static void print_version(void)
{
	printf("%s: %s\n", program_invocation_short_name, PACKAGE_STRING);
	printf("FUSE library version %s\n", fuse_pkgversion());
	fuse_lowlevel_version();
	exit(0);
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
		case 'v':
			++verbose;
			break;
		case 'f':
			hsfs_opts->fg = 1;
			ret = 1;
			break;
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
