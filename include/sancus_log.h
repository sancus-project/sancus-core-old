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
#ifndef _SANCUS_LOG_H
#define _SANCUS_LOG_H

enum sancus_log_level {
	SANCUS_LOG_EMERG,
	SANCUS_LOG_ALERT,
	SANCUS_LOG_CRIT,
	SANCUS_LOG_ERR,
	SANCUS_LOG_WARNING,
	SANCUS_LOG_NOTICE,
	SANCUS_LOG_INFO,
	SANCUS_LOG_TRACE,
	SANCUS_LOG_DEBUG,
};

void sancus_log_write(enum sancus_log_level level, const char *name, const char *str);
void sancus_log_writef(enum sancus_log_level level, const char *name, const char *fmt, ...)
		       TYPECHECK_PRINTF(3, 4);

#define _err(L, S)	sancus_log_write(L, LOG_PREFIX, S)
#define _errf(L, ...)	sancus_log_writef(L, LOG_PREFIX, __VA_ARGS__)

void sancus_log_trace(enum sancus_log_level level, const char *name,
		      const char *file, unsigned line, const char *func,
		      const char *str);
void sancus_log_tracef(enum sancus_log_level level, const char *name,
		       const char *file, unsigned line, const char *func,
		       const char *fmt, ...)
		       TYPECHECK_PRINTF(6, 7);

#define _trace(L, S)	sancus_log_trace(L, LOG_PREFIX, __FILE__, __LINE__, __func__, S)
#define _tracef(L, ...)	sancus_log_tracef(L, LOG_PREFIX, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define info(S)		_err(SANCUS_LOG_INFO, S)
#define warn(S)		_err(SANCUS_LOG_WARNING, S)
#define err(S)		_err(SANCUS_LOG_INFO, S)

#define infof(F, ...)	_errf(SANCUS_LOG_INFO, F, __VA_ARGS__)
#define warnf(F, ...)	_errf(SANCUS_LOG_WARNING, F, __VA_ARGS__)
#define errf(F, ...)	_errf(SANCUS_LOG_ERR, F, __VA_ARGS__)

#define syserr(S)	errf(S ": %s", strerror(errno))
#define syserrf(F, ...)	errf(F ": %s", __VA_ARGS__, strerror(errno))

#ifndef NDEBUG
#define debug(S)	_trace(SANCUS_LOG_DEBUG, S)
#define debugf(F, ...)	_tracef(SANCUS_LOG_DEBUG, F, __VA_ARGS__)
#else
#define debug(S)	((void)0)
#define debugf(...)	((void)0)
#endif

#endif
