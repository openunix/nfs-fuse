/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2019  Red Hat, Inc.

  Logging API.

  This program can be distributed under the terms of the GNU LGPLv2.
  See the file COPYING.LIB
*/

#include "hsfs_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <syslog.h>

#define MAX_SYSLOG_LINE_LEN 512

static bool to_syslog = false;

static void default_log_func(__attribute__((unused)) enum hsfs_log_level level,
			     const char *fmt, va_list ap)
{
	if (to_syslog) {
		int sys_log_level = LOG_ERR;

		/*
		 * with glibc hsfs_log_level has identical values as
		 * syslog levels, but we also support BSD - better we convert to
		 * be sure.
		 */
		switch (level) {
		case hsfs_LOG_DEBUG:
			sys_log_level = LOG_DEBUG;
			break;
		case hsfs_LOG_INFO:
			sys_log_level = LOG_INFO;
			break;
		case hsfs_LOG_NOTICE:
			sys_log_level = LOG_NOTICE;
			break;
		case hsfs_LOG_WARNING:
			sys_log_level = LOG_WARNING;
			break;
		case hsfs_LOG_ERR:
			sys_log_level = LOG_ERR;
			break;
		case hsfs_LOG_CRIT:
			sys_log_level = LOG_CRIT;
			break;
		case hsfs_LOG_ALERT:
			sys_log_level = LOG_ALERT;
			break;
		case hsfs_LOG_EMERG:
			sys_log_level = LOG_EMERG;
		}

		char log[MAX_SYSLOG_LINE_LEN];
		vsnprintf(log, MAX_SYSLOG_LINE_LEN, fmt, ap);
		syslog(sys_log_level, "%s", log);
	} else {
		vfprintf(stderr, fmt, ap);
	}
}

static hsfs_log_func_t log_func = default_log_func;

void hsfs_set_log_func(hsfs_log_func_t func)
{
	if (!func)
		func = default_log_func;

	log_func = func;
}

void hsfs_log(enum hsfs_log_level level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_func(level, fmt, ap);
	va_end(ap);
}

void hsfs_log_enable_syslog(const char *ident, int option, int facility)
{
	to_syslog = true;

	openlog(ident, option, facility);
}

void hsfs_log_close_syslog(void)
{
	closelog();
}
