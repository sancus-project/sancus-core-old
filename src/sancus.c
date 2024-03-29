/*
 * This file is part of sancus-core <http://github.com/amery/sancus-core>>
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

#include <ev.h>

#include "sancus.h"
#include "sancus_list.h"
#include "sancus_state.h"
#include "sancus_signal.h"

static struct sancus_state default_state;
static struct sancus_signal_watcher default_signal_watchers[2];

/*
 */
int _stop(struct sancus_state *s, int signum)
{
	assert(s == &default_state);

	(void)s;
	(void)signum;

	return 1;
}

/*
 * Default State
 */
SancusState sancus_init(void)
{
	SancusState s = &default_state;
	if (!s->loop) {
		sancus_state_init(s, ev_default_loop(0));
		sancus_signal_watcher_add2(&default_signal_watchers[0], s,
					   SIGINT, _stop);
		sancus_signal_watcher_add2(&default_signal_watchers[1], s,
					   SIGTERM, _stop);
	}

	return s;
}

SancusState sancus_default_state(void)
{
	assert(default_state.loop);
	return &default_state;
}

void sancus_finish(void)
{
	/* NOP */
}

void sancus_run(void)
{
	sancus_state_run(&default_state);
}
