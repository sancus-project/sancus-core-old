#!/bin/sh

mkdir -p "${0%/*}/m4"
exec autoreconf -ivs "${0%/*}"
