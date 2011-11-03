AC_INIT(sancus-core, 0.0, amery@geeks.cl)

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign dist-bzip2 -Wall -Werror])

AC_CONFIG_MACRO_DIR([m4])

LT_INIT

#
# Check for programs
#

AC_PROG_CC_C99

#
# Check for libraries
#

PKG_CHECK_MODULES(libev, [libev])

AC_CONFIG_FILES([sancus-core.pc])
AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 src/Makefile])
AC_OUTPUT
