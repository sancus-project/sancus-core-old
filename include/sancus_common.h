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
#ifndef _SANCUS_COMMON_H
#define _SANCUS_COMMON_H

/** Not used function argument */
#ifdef UNUSED
#elif defined(__GNUC__)
#	define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#else
#	define UNUSED(x) x
#endif

/** Reference to the structure based on the address of one of it's components */
#ifndef container_of
#define container_of(P,T,M)	(T *)((char *)(P) - offsetof(T, M))
#endif

/** likely to be 1 */
#ifdef likely
#elif defined(__GNUC__)
#	define likely(e)	__builtin_expect((e), 1)
#else
#	define likely(e)	(e)
#endif

/** likely to be 0 */
#ifdef unlikely
#elif defined(__GNUC__)
#	define unlikely(e)	__builtin_expect((e), 0)
#else
#	define unlikely(e)	(e)
#endif

/** Number of elements of an array */
#ifndef ELEMENTS
#define ELEMENTS(A)	(sizeof(A)/sizeof((A)[0]))
#endif

/** Check printf format */
#ifdef TYPECHECK_PRINTF
#elif defined(__GNUC__)
#       define TYPECHECK_PRINTF(I, J)   __attribute__ ((format (printf, I, J)))
#else
#       define TYPECHECK_PRINTF(I, J)
#endif

#endif /* !_SANCUS_COMMON_H */
