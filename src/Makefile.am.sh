#!/bin/sh

list() {
	ls -1 "$@" | sort -V | tr '\n' ' ' | fmt -w60 | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
	}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
AM_CFLAGS = -I @top_srcdir@/include \$(EV_CFLAGS)

lib_LTLIBRARIES = libsancus.la

libsancus_la_SOURCES = \\
	$(list *.c)
EOT
