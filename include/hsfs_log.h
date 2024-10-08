/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2019  Red Hat, Inc.

  This program can be distributed under the terms of the GNU LGPLv2.
  See the file COPYING.LIB.
*/

#ifndef hsfs_LOG_H_
#define hsfs_LOG_H_

/** @file
 *
 * This file defines the logging interface of hsfs
 */

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Log severity level
 *
 * These levels correspond to syslog(2) log levels since they are widely used.
 */
enum hsfs_log_level {
	hsfs_LOG_EMERG,
	hsfs_LOG_ALERT,
	hsfs_LOG_CRIT,
	hsfs_LOG_ERR,
	hsfs_LOG_WARNING,
	hsfs_LOG_NOTICE,
	hsfs_LOG_INFO,
	hsfs_LOG_DEBUG
};

/**
 * Log message handler function.
 *
 * This function must be thread-safe.  It may be called from any libhsfs
 * function, including hsfs_parse_cmdline() and other functions invoked before
 * a hsfs filesystem is created.
 *
 * Install a custom log message handler function using hsfs_set_log_func().
 *
 * @param level log severity level
 * @param fmt sprintf-style format string including newline
 * @param ap format string arguments
 */
typedef void (*hsfs_log_func_t)(enum hsfs_log_level level,
				const char *fmt, va_list ap);

/**
 * Install a custom log handler function.
 *
 * Log messages are emitted by libhsfs functions to report errors and debug
 * information.  Messages are printed to stderr by default but this can be
 * overridden by installing a custom log message handler function.
 *
 * The log message handler function is global and affects all hsfs filesystems
 * created within this process.
 *
 * @param func a custom log message handler function or NULL to revert to
 *             the default
 */
void hsfs_set_log_func(hsfs_log_func_t func);

/**
 * Emit a log message
 *
 * @param level severity level (hsfs_LOG_ERR, hsfs_LOG_DEBUG, etc)
 * @param fmt sprintf-style format string including newline
 */
void hsfs_log(enum hsfs_log_level level, const char *fmt, ...);

/**
 * Switch default log handler from stderr to syslog
 *
 * Passed options are according to 'man 3 openlog'
 */
void hsfs_log_enable_syslog(const char *ident, int option, int facility);

/**
 * To be called at teardown to close syslog.
*/
void hsfs_log_close_syslog(void);

#ifdef __cplusplus
}
#endif

#endif /* hsfs_LOG_H_ */
