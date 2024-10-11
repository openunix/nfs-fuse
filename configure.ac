#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([nfs-fuse], [0.3.3], [steve.shuo.feng@gmail.com])
AC_CONFIG_SRCDIR([fuse/hsfs_main.c])
AC_CONFIG_HEADERS([include/config.h])

AM_INIT_AUTOMAKE

# SLES 11SP2 (automake-1.10.1) doesn't have this
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

TIRPC_HEADERS="-I /usr/include/tirpc"
AC_SUBST([TIRPC_HEADERS])
AC_ARG_WITH([tirpc-headers],
	[AC_HELP_STRING([--with-tirpc-headers=<path>], [Alternative path to tirpc headers])],
	[TIRPC_HEADERS="-I $withval"
	case $BWLABELDIR in
     	     /*) ;;
     	     *) BWLABELDIR="`pwd`/$BWLABELDIR" ;;
	esac])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

AC_GNU_SOURCE

AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([fuse3], [fuse_session_mount], [], AC_MSG_ERROR([You need libfuse3 to run.]))
AC_CHECK_LIB([tirpc], [clnt_tp_create], [], AC_MSG_ERROR([You need libtirpc to run.]))

AC_CHECK_LIB([nfs], [rpc_nfs4_compound_async], [], AC_MSG_ERROR([You need libnfs to run.]))

FUSE_CFLAGS=`pkg-config fuse3 --cflags`
AC_SUBST(FUSE_CFLAGS)

# Checks for header files.
AC_HEADER_ASSERT
AC_HEADER_DIRENT
AC_HEADER_MAJOR
AC_HEADER_RESOLV
AC_HEADER_STAT
AC_HEADER_STDBOOL

AC_CHECK_HEADERS([arpa/inet.h fcntl.h libintl.h limits.h memory.h mntent.h netdb.h netinet/in.h paths.h stdlib.h string.h sys/mount.h sys/socket.h sys/time.h syslog.h unistd.h])

# Required by the built-in libnfs
AC_CHECK_HEADERS([sys/sysmacros.h utime.h net/if.h sys/statvfs.h])

AC_CHECK_HEADERS([nfsc/libnfs-raw.h])

# check for sockaddr_storage
dnl Check if sockaddr structure includes a "ss_family"
AC_CHECK_MEMBER([struct sockaddr_storage.ss_family],
                [ AC_DEFINE(HAVE_SOCKADDR_STORAGE,1,[Whether we have sockaddr_Storage]) ],
                [],
                [
#include <sys/types.h>
#include <sys/socket.h>
])

# Checks for types.
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T


# Checks for structures
AC_CHECK_MEMBERS([struct stat.st_atim])
AC_CHECK_MEMBERS([struct stat.st_atimespec])

# Check for compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([alarm gethostbyname hasmntopt inet_ntoa memset realpath socket strchr strcspn strdup strerror strrchr strstr uname])

# Check for system service
AC_SYS_LARGEFILE

#
# If we're running GCC or clang define _U_ to be "__attribute__((unused))"
# so we can use _U_ to flag unused function parameters and not get warnings
# about them. Otherwise, define _U_ to be an empty string so that _U_ used
# to flag an unused function parameters will compile with other compilers.
#
# XXX - similar hints for other compilers?
#
if test "x$GCC" = "xyes" -o "x$CC" = "xclang" ; then
  AC_DEFINE(_U_, __attribute__((unused)), [Hint to the compiler that a function parameters is not used])
else
  AC_DEFINE(_U_, , [Hint to the compiler that a function parameters is not used])
fi

AC_CONFIG_FILES([Makefile
		 include/Makefile
                 fuse/Makefile
                 nfs3/Makefile
                 man/Makefile
		 lib/Makefile
		 nfs_common/Makefile
		 utils/Makefile
		])
AC_OUTPUT
