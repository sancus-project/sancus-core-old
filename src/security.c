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

#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <paths.h>

#include "sancus.h"

#ifndef OPEN_MAX
#define	OPEN_MAX sysconf(_SC_OPEN_MAX)
#endif

/*
 * See O'Reilly's Secure Programming Cookbook (0-596-00394-3)
 * 1.5 Managing File Descriptors Safely
 */
static inline int reopen_devnull(int fd) {
	FILE *f = NULL;
	FILE *f0 = NULL;
	const char *mode = "wb";

	switch(fd) {
	case 0:
		f0 = stdin;
		mode = "rb";
		break;
	case 1:
		f0 = stdout;
		break;
	case 2:
		f0 = stderr;
		break;
	};

	if (f0)
		f = freopen(_PATH_DEVNULL, mode, f0);

	return (f && fileno(f) == fd);
}

void sancus_sanitize_files(void) {
	int fd, fds;
	struct stat st;

	if ((fds=getdtablesize()) == -1)
		fds = OPEN_MAX;
	for (fd=3; fd<fds; fd++)
		close(fd);

	for (fd=0; fd<3; fd++)
		if (fstat(fd, &st) == -1 &&
		    (errno != EBADF || !reopen_devnull(fd)))
			abort( );
}
