/*
 * This file is part of sancus-core <http://github.com/amery/sancus-core>
 *
 * Copyright (c) 2010, Alejandro Mery <amery@geeks.cl>
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "sancus_common.h"
#include "sancus_socket.h"

int sancus_socket(int family, int type, bool cloexec, bool nonblock)
{
	int fd;
#ifdef SOCK_CLOEXEC
	if (cloexec)
		type |= SOCK_CLOEXEC;
	else
		type &= ~SOCK_CLOEXEC;
#endif
#ifdef SOCK_NONBLOCK
	if (nonblock)
		type |= SOCK_NONBLOCK;
	else
		type &= ~SOCK_NONBLOCK;
#endif
	if ((fd = socket(family, type, 0)) < 0)
		goto socket_done;

	if (cloexec || nonblock) {
		int fl2, fl = fcntl(fd, F_GETFL);
		if (fl < 0)
			goto socket_failed;
		fl2 = fl;
		if (cloexec)
			fl2 |= FD_CLOEXEC;
		if (nonblock)
			fl2 |= O_NONBLOCK;

		if (fl != fl2 && fcntl(fd, F_SETFL, fl2) < 0)
			goto socket_failed;
		goto socket_done;
	}

socket_failed:
	close(fd);
	fd = -1;
socket_done:
	return fd;
}

static inline ssize_t sancus_socket_ntop_ipv4(char *str, size_t size, const struct sockaddr_in *sin)
{
	/* addr:port */
	if (inet_ntop(AF_INET, &sin->sin_addr, str, size) != NULL) {
		char port[8]; /* :123456\0 */
		size_t l = strlen(str);
		size_t lp = snprintf(port, sizeof(port), ":%u", ntohs(sin->sin_port));

		if (l + lp < size) {
			memcpy(str+l, port, lp+1);
			return l + lp;
		}
		errno = ENOSPC;
	}
	return -1;
}

static inline ssize_t sancus_socket_ntop_ipv6(char *str, size_t size,
					  const struct sockaddr_in6 *sin6)
{
	/* [addr]:port - http://www.ietf.org/rfc/rfc2732.txt */
	if (inet_ntop(AF_INET6, &sin6->sin6_addr, str+1, size-1) != NULL) {
		char port[8]; /* :123456\0 */
		size_t l = strlen(str+1);
		size_t lp = snprintf(port, sizeof(port), ":%u", ntohs(sin6->sin6_port));

		str[0] = '[';
		str[++l] = ']'; /* str[l+1] */
		l++; /* l+=2; */

		if (l + lp < size) {
			memcpy(str+l, port, lp+1);
			return l + lp;
		}
		errno = ENOSPC;
	}
	return -1;
}

ssize_t sancus_socket_ntop(char *str, size_t size, const struct sockaddr *sa,
		       socklen_t UNUSED(sa_len))
{
	switch (sa->sa_family) {
	case AF_INET:
		return sancus_socket_ntop_ipv4(str, size, (struct sockaddr_in *)sa);
	case AF_INET6:
		return sancus_socket_ntop_ipv6(str, size, (struct sockaddr_in6 *)sa);
	default:
		errno = EAFNOSUPPORT;
		return -1;
	}
}
