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
#ifndef _SANCUS_FD_H
#define _SANCUS_FD_H

static inline int sancus_open(const char *pathname, int flags, int cloexec, mode_t mode)
{
	int fd;
#ifdef O_CLOEXEC
	if (cloexec)
		flags |= O_CLOEXEC;
	else
		flags &= ~O_CLOEXEC;
#endif
try_open:
	if ((fd = open(pathname, flags, mode)) < 0) {
		if (errno == EINTR)
			goto try_open;
		else
			goto open_done;
	}

	if (cloexec) {
		int fl = fcntl(fd, F_GETFL);
		if (fl < 0)
			goto open_failed;
#ifdef O_CLOEXEC
		if ((fl & FD_CLOEXEC) == 0)
#endif
			if (fcntl(fd, F_SETFL, fl|FD_CLOEXEC) < 0) {
				goto open_failed;
			}
	}
	goto open_done;

open_failed:
	close(fd);
	fd = -1;
open_done:
	return fd;
}

static inline int sancus_close(int *fd)
{
	int ret = 0;
	if (fd != NULL && *fd >= 0) {
try_close:
		ret = close(*fd);
		if (ret == -1 && errno == EINTR)
			goto try_close;

		*fd = -1;
	}
	return ret;
}

static inline ssize_t sancus_write(int fd, const char *data, size_t size)
{
	int wc, wt = 0;

	while (size) {
try_write:
		wc = write(fd, data, size);
		if (unlikely(wc < 0)) {
			if (errno == EINTR)
				goto try_write;
			return wc;
		}

		wt += wc;
		size -= wc;
		data += wc;
	}

	return wt;
}

#ifdef _SYS_UIO_H
static inline ssize_t sancus_writev(int fd, struct iovec *iov, int iovcnt)
{
	int wt = 0;

	while (iovcnt) {
		register int wc;
try_write:
		wc = writev(fd, iov, iovcnt);
		if (unlikely(wc < 0)) {
			if (errno == EINTR)
				goto try_write;
			return wc;
		}

		wt += wc;
		while (wc) {
			if ((unsigned)wc >= iov->iov_len) {
				wc -= iov->iov_len;
				iovcnt--;
				iov++;
			} else {
				iov->iov_len -= wc;
				/* GCC: warning: pointer of type ‘void *’ used in arithmetic */
				iov->iov_base = (char*)iov->iov_base+wc;
				wc = 0;
			}
		}
	}

	return wt;
}
#endif /* _SYS_UIO_H */

#endif /* !_SANCUS_FD_H */
