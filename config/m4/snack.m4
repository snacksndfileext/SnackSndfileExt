#------------------------------------------------------------------------
# SC_PATH_SNACKCONFIG --
#
#	Locate the snackConfig.sh file and perform a sanity check on
#	the Snack compile flags
#
# Arguments:
#	none
#
# Results:
#
#	Adds the following arguments to configure:
#		--with-snack=...
#
#	Defines the following vars:
#		SNACK_BIN_DIR	Full path to the directory containing
#				the snackConfig.sh file
#------------------------------------------------------------------------

AC_DEFUN([SC_PATH_SNACKCONFIG], [
    #
    # Ok, lets find the snack configuration
    # First, look for one uninstalled.
    # the alternative search directory is invoked by --with-snack
    #

    if test x"${no_snack}" = x ; then
	# we reset no_snack in case something fails here
	no_snack=true
	AC_ARG_WITH(snack,
	    AC_HELP_STRING([--with-snack],
		[directory containing snack configuration (snackConfig.sh)]),
	    with_snackconfig="${withval}")
	AC_MSG_CHECKING([for Snack configuration])
	AC_CACHE_VAL(ac_cv_c_snackconfig,[

	    # First check to see if --with-snack was specified.
	    if test x"${with_snackconfig}" != x ; then
		case "${with_snackconfig}" in
		    */snackConfig.sh )
			if test -f "${with_snackconfig}"; then
			    AC_MSG_WARN([--with-snack argument should refer to directory containing snackConfig.sh, not to snackConfig.sh itself])
			    with_snackconfig="`echo "${with_snackconfig}" | sed 's!/snackConfig\.sh$!!'`"
			fi ;;
		esac
		if test -f "${with_snackconfig}/snackConfig.sh" ; then
		    ac_cv_c_snackconfig="`(cd "${with_snackconfig}"; pwd)`"
		else
		    AC_MSG_ERROR([${with_snackconfig} directory doesn't contain snackConfig.sh])
		fi
	    fi

	    # then check for a private Snack installation
	    if test x"${ac_cv_c_snackconfig}" = x ; then
		for i in \
			../snack \
			`ls -dr ../snack2.[[0-9]].[[0-9]]* 2>/dev/null` \
			`ls -dr ../snack2.[[0-9]] 2>/dev/null` \
			`ls -dr ../snack2.[[0-9]]* 2>/dev/null` \
			../../snack \
			`ls -dr ../../snack2.[[0-9]].[[0-9]]* 2>/dev/null` \
			`ls -dr ../../snack2.[[0-9]] 2>/dev/null` \
			`ls -dr ../../snack2.[[0-9]]* 2>/dev/null` \
			../../../snack \
			`ls -dr ../../../snack2.[[0-9]].[[0-9]]* 2>/dev/null` \
			`ls -dr ../../../snack2.[[0-9]] 2>/dev/null` \
			`ls -dr ../../../snack2.[[0-9]]* 2>/dev/null` ; do
		    if test -f "$i/unix/snackConfig.sh" ; then
			ac_cv_c_snackconfig="`(cd $i/unix; pwd)`"
			break
		    fi
		done
	    fi

	    # on Darwin, check in Framework installation locations
	    if test "`uname -s`" = "Darwin" -a x"${ac_cv_c_snackconfig}" = x ; then
		for i in `ls -d ~/Library/Frameworks 2>/dev/null` \
			`ls -d /Library/Frameworks 2>/dev/null` \
			`ls -d /Network/Library/Frameworks 2>/dev/null` \
			`ls -d /System/Library/Frameworks 2>/dev/null` \
			; do
		    if test -f "$i/Snack.framework/snackConfig.sh" ; then
			ac_cv_c_snackconfig="`(cd $i/Snack.framework; pwd)`"
			break
		    fi
		done
	    fi

	    # check in a few common install locations
	    if test x"${ac_cv_c_snackconfig}" = x ; then
		for i in `ls -d ${libdir} 2>/dev/null` \
			`ls -d ${exec_prefix}/lib 2>/dev/null` \
			`ls -d ${prefix}/lib 2>/dev/null` \
			`ls -d /usr/local/lib 2>/dev/null` \
			`ls -d /usr/contrib/lib 2>/dev/null` \
			`ls -d /usr/share/snacktk/snack8.5 2>/dev/null` \
			`ls -d /usr/lib 2>/dev/null` \
			`ls -d /usr/lib64 2>/dev/null` \
			; do
		    if test -f "$i/snackConfig.sh" ; then
			ac_cv_c_snackconfig="`(cd $i; pwd)`"
			break
		    fi
		done
	    fi

	    # check in a few other private locations
	    if test x"${ac_cv_c_snackconfig}" = x ; then
		for i in \
			${srcdir}/../snack \
			`ls -dr ${srcdir}/../snack2.[[0-9]].[[0-9]]* 2>/dev/null` \
			`ls -dr ${srcdir}/../snack2.[[0-9]] 2>/dev/null` \
			`ls -dr ${srcdir}/../snack2.[[0-9]]* 2>/dev/null` ; do
		    if test -f "$i/unix/snackConfig.sh" ; then
		    ac_cv_c_snackconfig="`(cd $i/unix; pwd)`"
		    break
		fi
		done
	    fi
	])

	if test x"${ac_cv_c_snackconfig}" = x ; then
	    SNACK_BIN_DIR="# no Snack configs found"
	    AC_MSG_ERROR([Can't find Snack configuration definitions. Use --with-snack to specify a directory containing snackConfig.sh])
	else
	    no_snack=
	    SNACK_BIN_DIR="${ac_cv_c_snackconfig}"
	    AC_MSG_RESULT([found ${SNACK_BIN_DIR}/snackConfig.sh])
	fi
    fi
])




#------------------------------------------------------------------------
# SC_LOAD_SNACKCONFIG --
#
#	Load the snackConfig.sh file
#
# Arguments:
#
#	Requires the following vars to be set:
#		SNACK_BIN_DIR
#
# Results:
#
#	Subst the following vars:
#		SNACK_BIN_DIR
#		SNACK_SRC_DIR
#		SNACK_LIB_FILE
#
#------------------------------------------------------------------------

AC_DEFUN([SC_LOAD_SNACKCONFIG], [
    AC_MSG_CHECKING([for existence of ${SNACK_BIN_DIR}/snackConfig.sh])

    if test -f "${SNACK_BIN_DIR}/snackConfig.sh" ; then
        AC_MSG_RESULT([loading])
	. "${SNACK_BIN_DIR}/snackConfig.sh"
    else
        AC_MSG_RESULT([could not find ${SNACK_BIN_DIR}/snackConfig.sh])
    fi

    # eval is required to do the SNACK_DBGX substitution
    eval "SNACK_LIB_FILE=\"${SNACK_LIB_FILE}\""
    eval "SNACK_STUB_LIB_FILE=\"${SNACK_STUB_LIB_FILE}\""

    # If the SNACK_BIN_DIR is the build directory (not the install directory),
    # then set the common variable name to the value of the build variables.
    # For example, the variable SNACK_LIB_SPEC will be set to the value
    # of SNACK_BUILD_LIB_SPEC. An extension should make use of SNACK_LIB_SPEC
    # instead of SNACK_BUILD_LIB_SPEC since it will work with both an
    # installed and uninstalled version of Snack.
    if test -f "${SNACK_BIN_DIR}/Makefile" ; then
        SNACK_LIB_SPEC="${SNACK_BUILD_LIB_SPEC}"
        SNACK_STUB_LIB_SPEC="${SNACK_BUILD_STUB_LIB_SPEC}"
        SNACK_STUB_LIB_PATH="${SNACK_BUILD_STUB_LIB_PATH}"
    elif test "`uname -s`" = "Darwin"; then
	# If Snack was built as a framework, attempt to use the libraries
	# from the framework at the given location so that linking works
	# against Snack.framework installed in an arbitrary location.
	case ${SNACK_DEFS} in
	    *SNACK_FRAMEWORK*)
		if test -f "${SNACK_BIN_DIR}/${SNACK_LIB_FILE}"; then
		    for i in "`cd "${SNACK_BIN_DIR}"; pwd`" \
			     "`cd "${SNACK_BIN_DIR}"/../..; pwd`"; do
			if test "`basename "$i"`" = "${SNACK_LIB_FILE}.framework"; then
			    SNACK_LIB_SPEC="-F`dirname "$i" | sed -e 's/ /\\\\ /g'` -framework ${SNACK_LIB_FILE}"
			    break
			fi
		    done
		fi
		if test -f "${SNACK_BIN_DIR}/${SNACK_STUB_LIB_FILE}"; then
		    SNACK_STUB_LIB_SPEC="-L`echo "${SNACK_BIN_DIR}"  | sed -e 's/ /\\\\ /g'` ${SNACK_STUB_LIB_FLAG}"
		    SNACK_STUB_LIB_PATH="${SNACK_BIN_DIR}/${SNACK_STUB_LIB_FILE}"
		fi
		;;
	esac
    fi
    for i in "`cd "${SNACK_BIN_DIR}"/..; pwd`" \
     	     ; do
     	if test -f "$i""/generic/snack.h"; then
     	    SNACK_INCLUDE_SPEC="-I""$i"/generic
     	    break
     	fi
     	if test -f "$i""/include/snack.h"; then
     	    SNACK_INCLUDE_SPEC="-I""$i"/include
     	    break
     	fi
     	if test -f "$i""/snack.h"; then
     	    SNACK_INCLUDE_SPEC="-I""$i"
     	    break
     	fi
    done
    SNACK_STUB_LIB_SPEC=-L"${SNACK_BIN_DIR}"" ""${SNACK_STUB_LIB_FLAG}"
    # eval is required to do the SNACK_DBGX substitution
    eval "SNACK_LIB_FLAG=\"${SNACK_LIB_FLAG}\""
    eval "SNACK_LIB_SPEC=\"${SNACK_LIB_SPEC}\""
    eval "SNACK_STUB_LIB_FLAG=\"${SNACK_STUB_LIB_FLAG}\""
    eval "SNACK_STUB_LIB_SPEC=\"${SNACK_STUB_LIB_SPEC}\""

    AC_SUBST(SNACK_VERSION)
    AC_SUBST(SNACK_PATCH_LEVEL)
    AC_SUBST(SNACK_BIN_DIR)
    AC_SUBST(SNACK_SRC_DIR)

    AC_SUBST(SNACK_LIB_FILE)
    AC_SUBST(SNACK_LIB_FLAG)
    AC_SUBST(SNACK_LIB_SPEC)

    AC_SUBST(SNACK_STUB_LIB_FILE)
    AC_SUBST(SNACK_STUB_LIB_FLAG)
    AC_SUBST(SNACK_STUB_LIB_SPEC)

    AC_SUBST(SNACK_INCLUDE_SPEC)
])

