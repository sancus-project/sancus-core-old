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
#include <stddef.h>	/* offsetof */
#include <stdlib.h>	/* NULL */

#include <ev.h>
#include "sancus.h"

#include "sancus_common.h"
#include "sancus_list.h"
#include "sancus_state.h"
#include "sancus_signal.h"

/*
 */
static void signal_callback(struct ev_loop *loop, ev_signal *w, int revents)
{
	struct sancus_signal_watcher *watcher = w->data;
	struct sancus_state *state = watcher->state;

	assert(state->loop == loop);

	sancus_list_foreach2(&watcher->handlers, item, next) {
		struct sancus_signal_handler *h = container_of(item, struct sancus_signal_handler,
								     handlers);
		h->h(state, w->signum);
	}

	if (watcher->h)
		watcher->h(state, w->signum);

	(void)loop;
	(void)revents;
}

static inline struct sancus_signal_watcher *_find_watcher(struct sancus_state *state,
							  int signum)
{
	sancus_list_foreach(&state->signal_watchers, item) {
		struct sancus_signal_watcher *watcher = container_of(item, struct sancus_signal_watcher,
								     watchers);
		if (watcher->w.signum == signum)
			return watcher;
	}
	return NULL;
}

/*
 * Exported
 */
int sancus_signal_watcher_add2(struct sancus_signal_watcher *self,
			       struct sancus_state *state,
			       int signum,
			       int (*h) (struct sancus_state *, int signum))
{
	assert(self);
	assert(state);
	assert(signum > 0 && signum <= 32);

	if (_find_watcher(state, signum))
		return 0;

	self->state = state;
	self->h = h;

	ev_signal_init(&self->w, signal_callback, signum);
	self->w.data = self;
	ev_signal_start(state->loop, &self->w);

	sancus_list_init(&self->handlers);

	sancus_list_init(&self->watchers);
	sancus_list_append(&state->signal_watchers, &self->watchers);

	return 1;
}

int sancus_signal_handler_add(struct sancus_signal_handler *self,
			      struct sancus_state *state,
			      int signum,
			      int (*h) (struct sancus_state *, int signum))
{
	struct sancus_signal_watcher *watcher;

	assert(self);
	assert(state);
	assert(signum > 0 && signum <= 32);
	assert(h);

	watcher = _find_watcher(state, signum);
	if (watcher)
		return sancus_signal_handler_add2(self, watcher, h);
	return 0;
}

int sancus_signal_handler_add2(struct sancus_signal_handler *self,
			       struct sancus_signal_watcher *watcher,
			       int (*h) (struct sancus_state *, int signum))
{
	assert(self);
	assert(watcher);
	assert(h);

	self->h = h;

	sancus_list_init(&self->handlers);
	sancus_list_append(&watcher->handlers, &self->handlers);
	return 0;
}
