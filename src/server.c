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

#include <unistd.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <ev.h>

#include "sancus_socket.h"
#include "sancus_server.h"

/*
 */
static void connect_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	if (revents & EV_READ) {
		struct sockaddr_storage addr;
		socklen_t addrlen = sizeof(addr);

		int fd = accept(w->fd, (struct sockaddr *)&addr, &addrlen);

		if (fd >= 0)
			close(fd);
	}

	(void)loop;
}

/*
 */
static inline int init_ipv4(struct sockaddr_in *sin, const char *addr, unsigned port)
{
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);

	/* NULL, "", "0" and "*" mean any address */
	if (addr == NULL || addr[0] == '\0' ||
	    ((addr[0] == '0' || addr[0] == '*') && addr[1] == '\0')) {
		sin->sin_addr.s_addr = htonl(INADDR_ANY);
		return 1;
	}

	return inet_pton(sin->sin_family, addr, &sin->sin_addr);
}

static inline int init_local(struct sockaddr_un *sun, const char *path)
{
	size_t l = 0;

	if (path == NULL) {
		path = "";
	} else {
		l = strlen(path);
		if (l > sizeof(sun->sun_path)-1)
			return 0; /* too long */
	}

	sun->sun_family = AF_LOCAL;
	memcpy(sun->sun_path, path, l+1);
	return 1;
}

static inline int init_tcp(int family, bool cloexec)
{
	int fd = sancus_socket(family, SOCK_STREAM, cloexec, true);

	if (fd >= 0 && family != AF_LOCAL) {
		int flags = 1;
		struct linger ling = {0,0}; /* disabled */

		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (void*)&ling, sizeof(ling));
	}

	return fd;
}

static inline void init_tcp_watcher(struct sancus_tcp_server *self, int fd)
{
	ev_io_init(&self->connection_watcher, connect_callback, fd, EV_READ);
	self->connection_watcher.data = self;
}

/*
 */
int sancus_tcp_ipv4_server(struct sancus_tcp_server *self, const char *addr, unsigned port,
			   bool cloexec)
{
	struct sockaddr_in sin;
	int fd, e;

	if ((e = init_ipv4(&sin, addr, port)) != 1)
		return e; /* 0 or -1, inet_pton() failed */

	if ((fd = init_tcp(sin.sin_family, cloexec)) < 0)
		return -1; /* socket() failed */
	else if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		close(fd);
		return -1; /* bind() failed */
	}

	init_tcp_watcher(self, fd);
	return 1;
}

int sancus_tcp_local_server(struct sancus_tcp_server *self, const char *path, bool cloexec)
{
	struct sockaddr_un sun;
	int fd, e;

	if ((e = init_local(&sun, path)) != 1)
		return e; /* 0 or -1 */
	unlink(path);

	if ((fd = init_tcp(sun.sun_family, cloexec)) < 0)
		return -1; /* socket() failed */
	else if (bind(fd, (struct sockaddr *)&sun, SUN_LEN(&sun)) < 0) {
		close(fd);
		return -1; /* bind() failed */
	}

	init_tcp_watcher(self, fd);
	return 1;
}
