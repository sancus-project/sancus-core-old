/*
 * This file is part of sancus-core <http://github.com/amery/sancus-core>
 *
 * Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>
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

#include <ev.h>

#include "sancus_list.h"
#include "sancus_server.h"

#include "server_connection.h"

/*
 */
static inline ssize_t _read(int fd, char *buf, size_t buf_len)
{
	ssize_t rc;
try_read:
	rc = read(fd, buf, buf_len);
	if (rc < 0 && errno == EINTR)
		goto try_read;
	return rc;
}

/*
 */
static void read_callback(struct ev_loop *loop, ev_io *w, int revents)
{
	struct sancus_tcp_server_connection *self = w->data;
	struct sancus_tcp_server *server = self->server;

	assert(server && server->loop == loop);

	if (revents & EV_READ) {
		char buf[4096];
		ssize_t rc = _read(w->fd, buf, sizeof(buf));

		if (rc > 0) {
			/* goto data */
		} else if (rc == 0) {
			/* EOF */
		} else {
			/* error */
		}
	}
}

void sancus_tcp_server_connection_prepare(struct sancus_tcp_server_connection *self,
				   struct sancus_tcp_server *server,
				   int fd)
{
	self->server = server;

	ev_io_init(&self->w, read_callback, fd, EV_READ);
	self->w.data = self;

	sancus_list_init(&self->connections);
	sancus_list_append(&server->connections, &self->connections);
}

void sancus_tcp_server_connection_start(struct sancus_tcp_server_connection *self, struct ev_loop *loop)
{
	ev_io_start(loop, &self->w);
}
