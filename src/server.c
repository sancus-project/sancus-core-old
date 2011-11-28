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
#include <errno.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <ev.h>

#include "sancus_list.h"
#include "sancus_socket.h"
#include "sancus_server.h"

#include "sancus_common.h"
#include "sancus_fd.h"

/*
 * tcp server
 */
void sancus_tcp_server_init(struct sancus_tcp_server *server)
{
	sancus_list_init(&server->ports);
	sancus_list_init(&server->connections);
}

/*
 */
static void connect_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	if (revents & EV_READ) {
		struct sockaddr_storage addr;
		socklen_t addrlen = sizeof(addr);

		int fd = accept(w->fd, (struct sockaddr *)&addr, &addrlen);

		if (fd >= 0)
			sancus_close(&fd);
	}

	(void)loop;
}

/*
 */
static inline int init_ipv4(struct sockaddr_in *sin, const char *addr, unsigned port)
{
	sin->sin_port = htons(port);

	/* NULL, "", "0" and "*" mean any address */
	if (addr == NULL || addr[0] == '\0' ||
	    ((addr[0] == '0' || addr[0] == '*') && addr[1] == '\0')) {
		sin->sin_addr.s_addr = htonl(INADDR_ANY);
		return 1;
	}

	return inet_pton(sin->sin_family, addr, &sin->sin_addr);
}

static inline int init_ipv6(struct sockaddr_in6 *sin6, const char *addr, unsigned port)
{
	sin6->sin6_port = htons(port);

	/* NULL, "", "::" and "*" mean any address */
	if (addr == NULL || addr[0] == '\0' ||
	    (addr[0] == '*' && addr[1] == '\0') ||
	    (addr[0] == ':' && addr[1] == ':' && addr[2] == '\0')) {
		sin6->sin6_addr = in6addr_any;
		return 1;
	}

	return inet_pton(sin6->sin6_family, addr, &sin6->sin6_addr);
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

	memcpy(sun->sun_path, path, l+1);
	return 1;
}

static inline int init_tcp(struct sancus_tcp_port *self, struct sancus_tcp_server *server,
			   struct sockaddr *sa, socklen_t sa_len,
			   bool cloexec, void (*sockopts) (int))
{
	int fd = sancus_socket(sa->sa_family, SOCK_STREAM, cloexec, true);
	if (fd < 0)
		return -1;

	if (sa->sa_family != AF_LOCAL) {
		int flags = 1;
		struct linger ling = {0,0}; /* disabled */

		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (void*)&ling, sizeof(ling));
	}
	if (sockopts)
		sockopts(fd);

	if (bind(fd, sa, sa_len) < 0) {
		int e = errno;
		sancus_close(&fd);
		errno = e;
		return -1;
	}

	ev_io_init(&self->connection_watcher, connect_callback, fd, EV_READ);
	self->connection_watcher.data = self;

	sancus_list_init(&self->ports);
	sancus_list_append(&server->ports, &self->ports);
	return 1;
}

/*
 */
int sancus_tcp_ipv4_port(struct sancus_tcp_port *self, struct sancus_tcp_server *server,
			 const char *addr, unsigned port,
			 bool cloexec, void (*sockopts) (int))
{
	struct sockaddr_in sin = { .sin_family = AF_INET };
	int e;

	if ((e = init_ipv4(&sin, addr, port)) != 1)
		return e; /* 0 or -1, inet_pton() failed */

	return init_tcp(self, server,
			(struct sockaddr *)&sin, sizeof(sin),
			cloexec, sockopts);
}


int sancus_tcp_ipv6_port(struct sancus_tcp_port *self, struct sancus_tcp_server *server,
			 const char *addr, unsigned port,
			 bool cloexec, void (*sockopts) (int))
{
	struct sockaddr_in6 sin6 = { .sin6_family = AF_INET6 };
	int e;

	if ((e = init_ipv6(&sin6, addr, port)) != 1)
		return e; /* 0 or -1, inet_pton() failed */

	return init_tcp(self, server,
			(struct sockaddr *)&sin6, sizeof(sin6),
			cloexec, sockopts);
}

int sancus_tcp_local_port(struct sancus_tcp_port *self, struct sancus_tcp_server *server,
				 const char *path,
				 bool cloexec, void (*sockopts) (int))
{
	struct sockaddr_un sun = { .sun_family = AF_LOCAL };
	int e;

	if ((e = init_local(&sun, path)) != 1)
		return e; /* 0 or -1 */
	unlink(path);

	return init_tcp(self, server,
			(struct sockaddr *)&sun, SUN_LEN(&sun),
			cloexec, sockopts);
}

int sancus_tcp_port_listen(struct sancus_tcp_port *self, unsigned backlog)
{
	return listen(self->connection_watcher.fd, backlog);
}

void sancus_tcp_port_close(struct sancus_tcp_port *self)
{
	int fd = self->connection_watcher.fd;

	sancus_close(&fd);
	sancus_list_del(&self->ports);
}

void sancus_tcp_start(struct sancus_tcp_port *self, struct ev_loop *loop)
{
	ev_io_start(loop, &self->connection_watcher);
}

void sancus_tcp_stop(struct sancus_tcp_port *self, struct ev_loop *loop)
{
	ev_io_stop(loop, &self->connection_watcher);
}
