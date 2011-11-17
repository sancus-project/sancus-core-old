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
#ifndef _SANCUS_LIST_H
#define _SANCUS_LIST_H

struct sancus_list {
	struct sancus_list *prev;
	struct sancus_list *next;
};

static inline void sancus_list_init(struct sancus_list *self)
{
	self->prev = self->next  = self;
}

static inline void sancus_list_inject(struct sancus_list *self, struct sancus_list *prev,
				      struct sancus_list *next)
{
	self->next = next;
	self->prev = prev;

	next->prev = self;
	prev->next = self;
}
#define sancus_list_insert(H, N)	sancus_list_inject(N, (H), (H)->next)
#define sancs_list_append(H, N)		sancus_list_inject(N, (H)->prev, (H))

static inline void __sancus_list_del(struct sancus_list *prev, struct sancus_list *next)
{
	next->prev = prev;
	prev->next = next;
}
#define sancus_list_del(S)	__sancus_list_del((S)->prev, (S)->next)

#define sancus_list_foreach(H, I) for(struct sancus_list *I = (H)->next; (I) != (H); (I) = (I)->next)
#define sancus_list_foreach2(H, I, N) for(struct sancus_list *I = (H)->next, *N = (I)->next; (I) != (H); (I) = (N), (N) = (I)->next)

#define sancus_list_isempty(H)	((H)->next == (H))
#define sancus_list_next(H, I)	((I)->next == (H) ? NULL : (I)->next)
#define sancus_list_first(H)	sancus_list_next(H, H)

#endif /* !_SANCUS_LIST_H */
