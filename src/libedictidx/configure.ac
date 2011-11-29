AC_INIT([edictidx], [0.01])
AC_CONFIG_AUX_DIR([config.aux])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects no-dependencies nostdinc])

LT_INIT
AC_CONFIG_MACRO_DIR([m4])

PACKAGE_BRIEF='"Small EDICT electronic dictionary indexing library"'
AC_SUBST([PACKAGE_BRIEF])

AC_PROG_CC
AC_PROG_RANLIB

AC_CHECK_HEADERS([unistd.h sys/types.h sys/stat.h sys/mman.h])
AC_CHECK_FUNCS([fileno stat ftruncate mmap munmap])

AM_CONDITIONAL([HAVE_MMAP], [test "${ac_cv_func_mmap}" == "yes"])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([docs/Doxyfile])])

AC_CHECK_PROGS([SPLINT], [splint])
AM_CONDITIONAL([HAVE_SPLINT], [test -n "$SPLINT"])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
