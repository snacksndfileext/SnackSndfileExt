# /* gcc $(pkg-config --cflags --libs sndfile) -lsnackstub2.2 -L/usr/lib/tcltk/snack2.2 -lsnack -I/usr/include/tcl8.4 -ltclstub -ltcl snack_sndfile_ext.c */
ACLOCAL_AMFLAGS=-I config/m4

versionedpkglibdir = $(pkglibdir)@VERSION@
versionedpkglib_DATA = pkgIndex.tcl
versionedpkglib_LTLIBRARIES = libsnack_sndfile_ext.la

libsnack_sndfile_ext_la_SOURCES = \
	snack_sndfile_ext.c
libsnack_sndfile_ext_la_CFLAGS = @SNACK_INCLUDE_SPEC@ @TCL_INCLUDE_SPEC@ @SNDFILE_CFLAGS@
libsnack_sndfile_ext_la_LDFLAGS = @SNACK_LIB_SPEC@ @TCL_LIB_SPEC@ @SNDFILE_LIBS@
#               TCL_STUB_LIB_FLAG
#               TCL_STUB_LIB_SPEC
#               TCL_STUB_LIB_FILE