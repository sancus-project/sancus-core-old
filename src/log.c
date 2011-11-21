/*
 * This file is part of sancus-core <http://github.com/amery/sancus-core>
 *
 * Copyright (c) 2010-2011, Alejandro Mery <amery@geeks.cl>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the author nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* sancus_fd.h */
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/uio.h>	/* writev() */

#include "sancus_common.h"

#include "sancus_fd.h"
#include "sancus_log.h"

/* arbitrary sizes */
#define STR_BUFSIZE 1024
#define TRACE_STR_BUFSIZE 1024

static inline ssize_t _write(enum sancus_log_level level, const char *name,
			     const char *trace_str, const char* str)
{
	struct iovec v[6];
	int l=0;
	char level_s[] = "<0> ";

	/* "<?> " */
	level_s[1] += level;
	v[l++] = (struct iovec) { level_s, 4 };

	/* "trace_str" */
	if (trace_str) {
		v[l++] = (struct iovec) { (void*)trace_str, (size_t)strlen(trace_str) };
	}

	/* "name: " */
	if (name) {
		v[l++] = (struct iovec) { (void*)name, (size_t)strlen(name) };
		v[l++] = (struct iovec) { ": ", 2 };
	}

	/* "str" */
	if (str) {
		v[l++] = (struct iovec) { (void*)str, (size_t)strlen(str) };
	}

	v[l++] = (struct iovec) { "\n", 1 };

	return sancus_writev(2, v, l);
}

void sancus_log_write(enum sancus_log_level level, const char *name, const char *str)
{
	_write(level, name, NULL, str);
}

void sancus_log_writef(enum sancus_log_level level, const char *name, const char *fmt, ...)
{
	va_list ap;
	char str[STR_BUFSIZE];

	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	_write(level, name, NULL, str);
}

void sancus_log_trace(enum sancus_log_level level, const char *name,
		      const char *file, unsigned line, const char *func,
		      const char *str)
{
	char trace_str[TRACE_STR_BUFSIZE];

	snprintf(trace_str, sizeof(trace_str), "%s:%u: %s: ", file, line, func);

	_write(level, name, trace_str, str);
}

void sancus_log_tracef(enum sancus_log_level level, const char *name,
		       const char *file, unsigned line, const char *func,
		       const char *fmt, ...)
{
	va_list ap;
	char str[STR_BUFSIZE];
	char trace_str[TRACE_STR_BUFSIZE];

	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	snprintf(trace_str, sizeof(trace_str), "%s:%u: %s: ", file, line, func);

	_write(level, name, trace_str, str);
}
