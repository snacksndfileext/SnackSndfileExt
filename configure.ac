AC_INIT([Snack SndFile Formats Extension], 0.0.1, [giuliopaci@gmail.com], snack_sndfile_ext)

dnl This file (configure.ac) revision.
AC_REVISION([$Revision: 0 $])

dnl The base directory (srcdir) must contain configure.ac.
AC_CONFIG_SRCDIR(configure.ac)

dnl Use config/ to store scripts and files used by configure.
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config/m4)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

SC_PATH_TCLCONFIG
dnl SC_PATH_TKCONFIG

SC_LOAD_TCLCONFIG
#       Subst the following vars:
#               TCL_BIN_DIR
#               TCL_SRC_DIR
#               TCL_LIB_FILE
#               TCL_LIB_FLAG
#               TCL_LIB_SPEC
#               TCL_STUB_LIB_FLAG
#               TCL_STUB_LIB_SPEC
#               TCL_STUB_LIB_FILE
AC_SUBST(TCL_INCLUDE_SPEC)
SC_CONFIG_CFLAGS

SC_ENABLE_SHARED

SC_PATH_SNACKCONFIG
SC_LOAD_SNACKCONFIG

PKG_CHECK_MODULES(SNDFILE, [sndfile], have_sndfile=true, have_sndfile=false)
AM_CONDITIONAL(SNDFILE, test x$have_sndfile = xtrue)
AC_SUBST(SNDFILE_CFLAGS)
AC_SUBST(SNDFILE_LIBS)



#--------------------------------------------------------------------
#	Disable stub support. This is necessary if you
#	want to run Snack with Tcl/Tk 8.0 up to 8.0.5
#--------------------------------------------------------------------

AC_ARG_ENABLE(stubs,[  --disable-stubs         disable stub support],,enableval2="no")

if test "$enableval2" = "no"; then
    AC_MSG_RESULT(will use stubs (normal))
    CFLAGS="$CFLAGS -DUSE_SNACK_STUBS -DUSE_TCL_STUBS $TCL_SHLIB_CFLAGS"
    TCL_LIB_SPEC="$TCL_STUB_LIB_SPEC"
    SNACK_LIB_SPEC="${SNACK_STUB_LIB_SPEC}"
    if test "$TCLVER" = "8.0"; then
        AC_MSG_WARN("You probably forgot to specify --disable-stubs")
    fi
else
    AC_MSG_RESULT(will not use stubs)
fi
#--------------------------------------------------------------------

AC_SUBST(CC)
AC_SUBST(CFLAGS)
AC_SUBST(TCL_LIB_SPEC)
AC_SUBST(SNACK_LIB_SPEC)
dnl Checks for header files.
AC_CHECK_HEADERS(string.h math.h locale.h)
AC_HEADER_STDC
dnl Checks for compiler characteristics.
dnl AC_ISC_POSIX
AC_C_INLINE
AM_PROG_CC_C_O
dnl Output.
AC_CONFIG_FILES([
Makefile
pkgIndex.tcl
])

AC_OUTPUT([])