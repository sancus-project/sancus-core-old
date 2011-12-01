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
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/socket.h>

#include <ev.h>

#include "sancus_list.h"
#include "sancus_server.h"

#include "sancus_common.h"
#include "sancus_fd.h"

#include "server_port.h"
#include "server_connection.h"

/*
 */
static void connect_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	struct sancus_tcp_port *port = w->data;
	struct sancus_tcp_server *server = port->server;
	struct sancus_tcp_server_connection *conn;

	assert(server && server->loop == loop);

	if (revents & EV_READ) {
		struct sockaddr_storage addr;
		socklen_t addrlen = sizeof(addr);

		int fd = accept(w->fd, (struct sockaddr *)&addr, &addrlen);

		if (fd >= 0) {
			conn = server->on_connect(server, fd,
						  (struct sockaddr *)&addr,
						  addrlen);
			if (conn) {
				sancus_tcp_server_connection_prepare(conn, server, fd);
				sancus_tcp_server_connection_start(conn, loop);
				return;
			}
			sancus_close(&fd);
		}
	}
}

void sancus_tcp_port_prepare(struct sancus_tcp_port *self, struct sancus_tcp_server *server, int fd)
{
	self->server = server;

	ev_io_init(&self->connection_watcher, connect_callback, fd, EV_READ);
	self->connection_watcher.data = self;

	sancus_list_init(&self->ports);
	sancus_list_append(&server->ports, &self->ports);

	if (server->loop)
		sancus_tcp_port_start(self, server->loop);
}

void sancus_tcp_port_close(struct sancus_tcp_port *self)
{
	int fd = self->connection_watcher.fd;

	sancus_close(&fd);
	sancus_list_del(&self->ports);
}
