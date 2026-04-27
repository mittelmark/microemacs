dnl $XTermId: aclocal.m4,v 1.108 2025/08/05 08:09:09 tom Exp $
dnl
dnl ---------------------------------------------------------------------------
dnl
dnl Copyright 2006-2024,2025 by Thomas E. Dickey
dnl
dnl                         All Rights Reserved
dnl
dnl Permission to use, copy, modify, and distribute this software and its
dnl documentation for any purpose and without fee is hereby granted,
dnl provided that the above copyright notice appear in all copies and that
dnl both that copyright notice and this permission notice appear in
dnl supporting documentation, and that the name of the above listed
dnl copyright holder(s) not be used in advertising or publicity pertaining
dnl to distribution of the software without specific, written prior
dnl permission.
dnl
dnl THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD
dnl TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
dnl AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE
dnl LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
dnl WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
dnl ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
dnl OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
dnl
dnl ---------------------------------------------------------------------------
dnl See
dnl     https://invisible-island.net/autoconf/autoconf.html
dnl     https://invisible-island.net/autoconf/my-autoconf.html
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl AM_ICONV version: 13 updated: 2024/10/16 03:56:13
dnl --------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl iconv.m4
dnl ====================
dnl serial AM2
dnl
dnl From Bruno Haible.
dnl
dnl ====================
dnl Modified to use CF_FIND_LINKAGE and CF_ADD_SEARCHPATH, to broaden the
dnl range of locations searched.  Retain the same cache-variable naming to
dnl allow reuse with the other gettext macros -Thomas E Dickey
AC_DEFUN([AM_ICONV],
[
  dnl Some systems have iconv in libc, some have it in libiconv (OSF/1 and
  dnl those with the standalone portable GNU libiconv installed).

  AC_ARG_WITH([libiconv-prefix],
[  --with-libiconv-prefix=DIR
                          search for libiconv in DIR/include and DIR/lib], [
    CF_ADD_OPTIONAL_PATH($withval, libiconv)
   ])

  AC_CACHE_CHECK(for iconv, am_cv_func_iconv, [
    CF_FIND_LINKAGE(CF__ICONV_HEAD,
      CF__ICONV_BODY,
      iconv,
      am_cv_func_iconv=yes,
      am_cv_func_iconv=["no, consider installing GNU libiconv"])])

  if test "$am_cv_func_iconv" = yes; then
    AC_DEFINE(HAVE_ICONV, 1, [Define if you have the iconv() function.])

    AC_CACHE_CHECK([if the declaration of iconv() needs const.],
		   am_cv_proto_iconv_const,[
      AC_TRY_COMPILE(CF__ICONV_HEAD [
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
size_t iconv (iconv_t cd, char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);
#else
size_t iconv();
#endif
],[], am_cv_proto_iconv_const=no,
      am_cv_proto_iconv_const=yes)])

    if test "$am_cv_proto_iconv_const" = yes ; then
      am_cv_proto_iconv_arg1="const"
    else
      am_cv_proto_iconv_arg1="/* nothing */"
    fi

    AC_DEFINE_UNQUOTED(ICONV_CONST, $am_cv_proto_iconv_arg1,
      [Define as const if the declaration of iconv() needs const.])
  fi

  LIBICONV=
  if test "$cf_cv_find_linkage_iconv" = yes; then
    CF_ADD_INCDIR($cf_cv_header_path_iconv)
    if test -n "$cf_cv_library_file_iconv" ; then
      LIBICONV="-liconv"
      CF_ADD_LIBDIR($cf_cv_library_path_iconv)
    fi
  fi

  AC_SUBST(LIBICONV)
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_LANGINFO_CODESET version: 7 updated: 2023/01/11 04:05:23
dnl -------------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl codeset.m4
dnl ====================
dnl serial AM1
dnl
dnl From Bruno Haible.
AC_DEFUN([AM_LANGINFO_CODESET],
[
AC_CACHE_CHECK([for nl_langinfo and CODESET], am_cv_langinfo_codeset,
	[AC_TRY_LINK([
$ac_includes_default
#include <langinfo.h>],
	[char* cs = nl_langinfo(CODESET); (void)cs],
	am_cv_langinfo_codeset=yes,
	am_cv_langinfo_codeset=no)
	])
	if test "$am_cv_langinfo_codeset" = yes; then
		AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
		[Define if you have <langinfo.h> and nl_langinfo(CODESET).])
	fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_CHECK version: 5 updated: 2014/06/04 19:11:49
dnl ------------------
dnl Conditionally generate script according to whether we're using a given autoconf.
dnl
dnl $1 = version to compare against
dnl $2 = code to use if AC_ACVERSION is at least as high as $1.
dnl $3 = code to use if AC_ACVERSION is older than $1.
define([CF_ACVERSION_CHECK],
[
ifdef([AC_ACVERSION], ,[ifdef([AC_AUTOCONF_VERSION],[m4_copy([AC_AUTOCONF_VERSION],[AC_ACVERSION])],[m4_copy([m4_PACKAGE_VERSION],[AC_ACVERSION])])])dnl
ifdef([m4_version_compare],
[m4_if(m4_version_compare(m4_defn([AC_ACVERSION]), [$1]), -1, [$3], [$2])],
[CF_ACVERSION_COMPARE(
AC_PREREQ_CANON(AC_PREREQ_SPLIT([$1])),
AC_PREREQ_CANON(AC_PREREQ_SPLIT(AC_ACVERSION)), AC_ACVERSION, [$2], [$3])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_COMPARE version: 3 updated: 2012/10/03 18:39:53
dnl --------------------
dnl CF_ACVERSION_COMPARE(MAJOR1, MINOR1, TERNARY1,
dnl                      MAJOR2, MINOR2, TERNARY2,
dnl                      PRINTABLE2, not FOUND, FOUND)
define([CF_ACVERSION_COMPARE],
[ifelse(builtin([eval], [$2 < $5]), 1,
[ifelse([$8], , ,[$8])],
[ifelse([$9], , ,[$9])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_CFLAGS version: 15 updated: 2020/12/31 10:54:15
dnl -------------
dnl Copy non-preprocessor flags to $CFLAGS, preprocessor flags to $CPPFLAGS
dnl $1 = flags to add
dnl $2 = if given makes this macro verbose.
dnl
dnl Put any preprocessor definitions that use quoted strings in $EXTRA_CPPFLAGS,
dnl to simplify use of $CPPFLAGS in compiler checks, etc., that are easily
dnl confused by the quotes (which require backslashes to keep them usable).
AC_DEFUN([CF_ADD_CFLAGS],
[
cf_fix_cppflags=no
cf_new_cflags=
cf_new_cppflags=
cf_new_extra_cppflags=

for cf_add_cflags in $1
do
case "$cf_fix_cppflags" in
(no)
	case "$cf_add_cflags" in
	(-undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C)
		case "$cf_add_cflags" in
		(-D*)
			cf_tst_cflags=`echo "${cf_add_cflags}" |sed -e 's/^-D[[^=]]*='\''\"[[^"]]*//'`

			test "x${cf_add_cflags}" != "x${cf_tst_cflags}" \
				&& test -z "${cf_tst_cflags}" \
				&& cf_fix_cppflags=yes

			if test "$cf_fix_cppflags" = yes ; then
				CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)
				continue
			elif test "${cf_tst_cflags}" = "\"'" ; then
				CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)
				continue
			fi
			;;
		esac
		case "$CPPFLAGS" in
		(*$cf_add_cflags)
			;;
		(*)
			case "$cf_add_cflags" in
			(-D*)
				cf_tst_cppflags=`echo "x$cf_add_cflags" | sed -e 's/^...//' -e 's/=.*//'`
				CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,$cf_tst_cppflags)
				;;
			esac
			CF_APPEND_TEXT(cf_new_cppflags,$cf_add_cflags)
			;;
		esac
		;;
	(*)
		CF_APPEND_TEXT(cf_new_cflags,$cf_add_cflags)
		;;
	esac
	;;
(yes)
	CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)

	cf_tst_cflags=`echo "${cf_add_cflags}" |sed -e 's/^[[^"]]*"'\''//'`

	test "x${cf_add_cflags}" != "x${cf_tst_cflags}" \
		&& test -z "${cf_tst_cflags}" \
		&& cf_fix_cppflags=no
	;;
esac
done

if test -n "$cf_new_cflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CFLAGS $cf_new_cflags)])
	CF_APPEND_TEXT(CFLAGS,$cf_new_cflags)
fi

if test -n "$cf_new_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CPPFLAGS $cf_new_cppflags)])
	CF_APPEND_TEXT(CPPFLAGS,$cf_new_cppflags)
fi

if test -n "$cf_new_extra_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$EXTRA_CPPFLAGS $cf_new_extra_cppflags)])
	CF_APPEND_TEXT(EXTRA_CPPFLAGS,$cf_new_extra_cppflags)
fi

AC_SUBST(EXTRA_CPPFLAGS)

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_INCDIR version: 17 updated: 2021/09/04 06:35:04
dnl -------------
dnl Add an include-directory to $CPPFLAGS.  Don't add /usr/include, since it is
dnl redundant.  We don't normally need to add -I/usr/local/include for gcc,
dnl but old versions (and some misinstalled ones) need that.  To make things
dnl worse, gcc 3.x may give error messages if -I/usr/local/include is added to
dnl the include-path).
AC_DEFUN([CF_ADD_INCDIR],
[
if test -n "$1" ; then
  for cf_add_incdir in $1
  do
	while test "$cf_add_incdir" != /usr/include
	do
	  if test -d "$cf_add_incdir"
	  then
		cf_have_incdir=no
		if test -n "$CFLAGS$CPPFLAGS" ; then
		  # a loop is needed to ensure we can add subdirs of existing dirs
		  for cf_test_incdir in $CFLAGS $CPPFLAGS ; do
			if test ".$cf_test_incdir" = ".-I$cf_add_incdir" ; then
			  cf_have_incdir=yes; break
			fi
		  done
		fi

		if test "$cf_have_incdir" = no ; then
		  if test "$cf_add_incdir" = /usr/local/include ; then
			if test "$GCC" = yes
			then
			  cf_save_CPPFLAGS=$CPPFLAGS
			  CF_APPEND_TEXT(CPPFLAGS,-I$cf_add_incdir)
			  AC_TRY_COMPILE([#include <stdio.h>],
				  [printf("Hello")],
				  [],
				  [cf_have_incdir=yes])
			  CPPFLAGS=$cf_save_CPPFLAGS
			fi
		  fi
		fi

		if test "$cf_have_incdir" = no ; then
		  CF_VERBOSE(adding $cf_add_incdir to include-path)
		  ifelse([$2],,CPPFLAGS,[$2])="$ifelse([$2],,CPPFLAGS,[$2]) -I$cf_add_incdir"

		  cf_top_incdir=`echo "$cf_add_incdir" | sed -e 's%/include/.*$%/include%'`
		  test "$cf_top_incdir" = "$cf_add_incdir" && break
		  cf_add_incdir="$cf_top_incdir"
		else
		  break
		fi
	  else
		break
	  fi
	done
  done
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIB version: 2 updated: 2010/06/02 05:03:05
dnl ----------
dnl Add a library, used to enforce consistency.
dnl
dnl $1 = library to add, without the "-l"
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIB],[CF_ADD_LIBS(-l$1,ifelse($2,,LIBS,[$2]))])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBDIR version: 11 updated: 2020/12/31 20:19:42
dnl -------------
dnl	Adds to the library-path
dnl
dnl	Some machines have trouble with multiple -L options.
dnl
dnl $1 is the (list of) directory(s) to add
dnl $2 is the optional name of the variable to update (default LDFLAGS)
dnl
AC_DEFUN([CF_ADD_LIBDIR],
[
if test -n "$1" ; then
	for cf_add_libdir in $1
	do
		if test "$cf_add_libdir" = /usr/lib ; then
			:
		elif test -d "$cf_add_libdir"
		then
			cf_have_libdir=no
			if test -n "$LDFLAGS$LIBS" ; then
				# a loop is needed to ensure we can add subdirs of existing dirs
				for cf_test_libdir in $LDFLAGS $LIBS ; do
					if test ".$cf_test_libdir" = ".-L$cf_add_libdir" ; then
						cf_have_libdir=yes; break
					fi
				done
			fi
			if test "$cf_have_libdir" = no ; then
				CF_VERBOSE(adding $cf_add_libdir to library-path)
				ifelse([$2],,LDFLAGS,[$2])="-L$cf_add_libdir $ifelse([$2],,LDFLAGS,[$2])"
			fi
		fi
	done
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBS version: 3 updated: 2019/11/02 16:47:33
dnl -----------
dnl Add one or more libraries, used to enforce consistency.  Libraries are
dnl prepended to an existing list, since their dependencies are assumed to
dnl already exist in the list.
dnl
dnl $1 = libraries to add, with the "-l", etc.
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIBS],[
cf_add_libs="[$]ifelse($2,,LIBS,[$2])"
# reverse order
cf_add_0lib=
for cf_add_1lib in $1; do cf_add_0lib="$cf_add_1lib $cf_add_0lib"; done
# filter duplicates
for cf_add_1lib in $cf_add_0lib; do
	for cf_add_2lib in $cf_add_libs; do
		if test "x$cf_add_1lib" = "x$cf_add_2lib"; then
			cf_add_1lib=
			break
		fi
	done
	test -n "$cf_add_1lib" && cf_add_libs="$cf_add_1lib $cf_add_libs"
done
ifelse($2,,LIBS,[$2])="$cf_add_libs"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_OPTIONAL_PATH version: 6 updated: 2024/09/10 19:19:44
dnl --------------------
dnl Add an optional search-path to the compile/link variables.
dnl See CF_WITH_PATH
dnl
dnl $1 = shell variable containing the result of --with-XXX=[DIR]
dnl $2 = module to look for.
AC_DEFUN([CF_ADD_OPTIONAL_PATH],[
case "x$1" in
(xno|xyes|x)
	;;
(*)
	CF_ADD_SEARCHPATH([$1], [AC_MSG_ERROR(cannot find $2 under $1)])
	CF_VERBOSE(setting path value for ifelse([$2],,variable,[$2]) to $1)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_SEARCHPATH version: 6 updated: 2020/12/31 20:19:42
dnl -----------------
dnl Set $CPPFLAGS and $LDFLAGS with the directories given via the parameter.
dnl They can be either the common root of include- and lib-directories, or the
dnl lib-directory (to allow for things like lib64 directories).
dnl See also CF_FIND_LINKAGE.
dnl
dnl $1 is the list of colon-separated directory names to search.
dnl $2 is the action to take if a parameter does not yield a directory.
AC_DEFUN([CF_ADD_SEARCHPATH],
[
AC_REQUIRE([CF_PATHSEP])
for cf_searchpath in `echo "$1" | tr $PATH_SEPARATOR ' '`; do
	if test -d "$cf_searchpath/include" ; then
		CF_ADD_INCDIR($cf_searchpath/include)
	elif test -d "$cf_searchpath/../include" ; then
		CF_ADD_INCDIR($cf_searchpath/../include)
	ifelse([$2],,,[else
$2])
	fi
	if test -d "$cf_searchpath/lib" ; then
		CF_ADD_LIBDIR($cf_searchpath/lib)
	elif test -d "$cf_searchpath" ; then
		CF_ADD_LIBDIR($cf_searchpath)
	ifelse([$2],,,[else
$2])
	fi
done
])
dnl ---------------------------------------------------------------------------
dnl CF_ADD_SUBDIR_PATH version: 5 updated: 2020/12/31 20:19:42
dnl ------------------
dnl Append to a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
dnl $4 = the directory under which we will test for subdirectories
dnl $5 = a directory that we do not want $4 to match
AC_DEFUN([CF_ADD_SUBDIR_PATH],
[
test "x$4" != "x$5" && \
test -d "$4" && \
ifelse([$5],NONE,,[{ test -z "$5" || test "x$5" = xNONE || test "x$4" != "x$5"; } &&]) {
	test -n "$verbose" && echo "	... testing for $3-directories under $4"
	test -d "$4/$3" &&          $1="[$]$1 $4/$3"
	test -d "$4/$3/$2" &&       $1="[$]$1 $4/$3/$2"
	test -d "$4/$3/$2/$3" &&    $1="[$]$1 $4/$3/$2/$3"
	test -d "$4/$2/$3" &&       $1="[$]$1 $4/$2/$3"
	test -d "$4/$2/$3/$2" &&    $1="[$]$1 $4/$2/$3/$2"
}
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_CHECK version: 15 updated: 2024/05/01 15:00:19
dnl ----------------
dnl This was originally adapted from the macros 'fp_PROG_CC_STDC' and
dnl 'fp_C_PROTOTYPES' in the sharutils 4.2 distribution.
AC_DEFUN([CF_ANSI_CC_CHECK],
[
CF_CC_ENV_FLAGS

AC_CACHE_CHECK(for ${CC:-cc} option to accept ANSI C, cf_cv_ansi_cc,[
cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" \
	"" \
	-qlanglvl=ansi \
	-std1 \
	-Ae \
	"-Aa -D_HPUX_SOURCE" \
	-Xc
do
	CFLAGS="$cf_save_CFLAGS"
	CPPFLAGS="$cf_save_CPPFLAGS"
	CF_ADD_CFLAGS($cf_arg)
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || (__STDC__ != 1)
choke me
#endif
#endif
extern int test (int i, double x);
],[
	struct s1 {int (*f) (int a);};
	struct s2 {int (*f) (double a);};],
	[cf_cv_ansi_cc="$cf_arg"; break])
done
CFLAGS="$cf_save_CFLAGS"
CPPFLAGS="$cf_save_CPPFLAGS"
])

if test "$cf_cv_ansi_cc" != "no"; then
if test ".$cf_cv_ansi_cc" != ".-DCC_HAS_PROTOS"; then
	CF_ADD_CFLAGS($cf_cv_ansi_cc)
else
	AC_DEFINE(CC_HAS_PROTOS,1,[Define to 1 if C compiler supports prototypes])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_REQD version: 4 updated: 2008/03/23 14:48:54
dnl ---------------
dnl For programs that must use an ANSI compiler, obtain compiler options that
dnl will make it recognize prototypes.  We'll do preprocessor checks in other
dnl macros, since tools such as unproto can fake prototypes, but only part of
dnl the preprocessor.
AC_DEFUN([CF_ANSI_CC_REQD],
[AC_REQUIRE([CF_ANSI_CC_CHECK])
if test "$cf_cv_ansi_cc" = "no"; then
	AC_MSG_ERROR(
[Your compiler does not appear to recognize prototypes.
You have the following choices:
	a. adjust your compiler options
	b. get an up-to-date compiler
	c. use a wrapper such as unproto])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_APPEND_CFLAGS version: 3 updated: 2021/09/05 17:25:40
dnl ----------------
dnl Use CF_ADD_CFLAGS after first checking for potential redefinitions.
dnl $1 = flags to add
dnl $2 = if given makes this macro verbose.
define([CF_APPEND_CFLAGS],
[
for cf_add_cflags in $1
do
	case "x$cf_add_cflags" in
	(x-[[DU]]*)
		CF_REMOVE_CFLAGS($cf_add_cflags,CFLAGS,[$2])
		CF_REMOVE_CFLAGS($cf_add_cflags,CPPFLAGS,[$2])
		;;
	esac
	CF_ADD_CFLAGS([$cf_add_cflags],[$2])
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_APPEND_TEXT version: 1 updated: 2017/02/25 18:58:55
dnl --------------
dnl use this macro for appending text without introducing an extra blank at
dnl the beginning
define([CF_APPEND_TEXT],
[
	test -n "[$]$1" && $1="[$]$1 "
	$1="[$]{$1}$2"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_ENABLE version: 3 updated: 1999/03/30 17:24:31
dnl -------------
dnl Allow user to enable a normally-off option.
AC_DEFUN([CF_ARG_ENABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],no)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 6 updated: 2025/08/05 04:09:09
dnl -------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string
dnl $3 = action to perform if option is not default
dnl $4 = action to perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE([$1],[$2],[test "$enableval" != ifelse([$5],no,yes,no) && enableval=ifelse([$5],no,no,yes)
	if test "$enableval" != "$5" ; then
ifelse([$3],,[    :]dnl
,[    $3]) ifelse([$4],,,[
	else
		$4])
	fi],[enableval=$5 ifelse([$4],,,[
	$4
])dnl
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_C11_NORETURN version: 4 updated: 2023/02/18 17:41:25
dnl ---------------
AC_DEFUN([CF_C11_NORETURN],
[
AC_MSG_CHECKING(if you want to use C11 _Noreturn feature)
CF_ARG_ENABLE(stdnoreturn,
	[  --enable-stdnoreturn    enable C11 _Noreturn feature for diagnostics],
	[enable_stdnoreturn=yes],
	[enable_stdnoreturn=no])
AC_MSG_RESULT($enable_stdnoreturn)

if test $enable_stdnoreturn = yes; then
AC_CACHE_CHECK([for C11 _Noreturn feature], cf_cv_c11_noreturn,
	[AC_TRY_COMPILE([
$ac_includes_default
#include <stdnoreturn.h>
static _Noreturn void giveup(void) { exit(0); }
	],
	[if (feof(stdin)) giveup()],
	cf_cv_c11_noreturn=yes,
	cf_cv_c11_noreturn=no)
	])
else
	cf_cv_c11_noreturn=no,
fi

if test "$cf_cv_c11_noreturn" = yes; then
	AC_DEFINE(HAVE_STDNORETURN_H, 1,[Define if <stdnoreturn.h> header is available and working])
	AC_DEFINE_UNQUOTED(STDC_NORETURN,_Noreturn,[Define if C11 _Noreturn keyword is supported])
	HAVE_STDNORETURN_H=1
else
	HAVE_STDNORETURN_H=0
fi

AC_SUBST(HAVE_STDNORETURN_H)
AC_SUBST(STDC_NORETURN)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CC_ENV_FLAGS version: 11 updated: 2023/02/20 11:15:46
dnl ---------------
dnl Check for user's environment-breakage by stuffing CFLAGS/CPPFLAGS content
dnl into CC.  This will not help with broken scripts that wrap the compiler
dnl with options, but eliminates a more common category of user confusion.
dnl
dnl In particular, it addresses the problem of being able to run the C
dnl preprocessor in a consistent manner.
dnl
dnl Caveat: this also disallows blanks in the pathname for the compiler, but
dnl the nuisance of having inconsistent settings for compiler and preprocessor
dnl outweighs that limitation.
AC_DEFUN([CF_CC_ENV_FLAGS],
[
# This should have been defined by AC_PROG_CC
: "${CC:=cc}"

AC_MSG_CHECKING(\$CFLAGS variable)
case "x$CFLAGS" in
(*-[[IUD]]*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment uses the CFLAGS variable to hold CPPFLAGS options)
	cf_flags="$CFLAGS"
	CFLAGS=
	for cf_arg in $cf_flags
	do
		CF_ADD_CFLAGS($cf_arg)
	done
	;;
(*)
	AC_MSG_RESULT(ok)
	;;
esac

AC_MSG_CHECKING(\$CC variable)
case "$CC" in
(*[[\ \	]]-*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment uses the CC variable to hold CFLAGS/CPPFLAGS options)
	# humor him...
	cf_prog=`echo "$CC" | sed -e 's/	/ /g' -e 's/[[ ]]* / /g' -e 's/[[ ]]*[[ ]]-[[^ ]].*//'`
	cf_flags=`echo "$CC" | sed -e "s%^$cf_prog%%"`
	CC="$cf_prog"
	for cf_arg in $cf_flags
	do
		case "x$cf_arg" in
		(x-[[IUDfgOW]]*)
			CF_ADD_CFLAGS($cf_arg)
			;;
		(*)
			CC="$CC $cf_arg"
			;;
		esac
	done
	CF_VERBOSE(resulting CC: '$CC')
	CF_VERBOSE(resulting CFLAGS: '$CFLAGS')
	CF_VERBOSE(resulting CPPFLAGS: '$CPPFLAGS')
	;;
(*)
	AC_MSG_RESULT(ok)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CACHE version: 13 updated: 2020/12/31 10:54:15
dnl --------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname).  Normally we'll use AC_CANONICAL_HOST, but allow
dnl an extra parameter that we may override, e.g., for AC_CANONICAL_SYSTEM
dnl which is useful in cross-compiles.
dnl
dnl Note: we would use $ac_config_sub, but that is one of the places where
dnl autoconf 2.5x broke compatibility with autoconf 2.13
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f "$srcdir/config.guess" || test -f "$ac_aux_dir/config.guess" ; then
	ifelse([$1],,[AC_CANONICAL_HOST],[$1])
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name",[Define to the system name.])
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_MSG_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_ERRNO version: 14 updated: 2023/02/18 17:41:25
dnl --------------
dnl Check for data that is usually declared in <stdio.h> or <errno.h>, e.g.,
dnl the 'errno' variable.  Define a DECL_xxx symbol if we must declare it
dnl ourselves.
dnl
dnl $1 = the name to check
dnl $2 = the assumed type
AC_DEFUN([CF_CHECK_ERRNO],
[
AC_CACHE_CHECK(if external $1 is declared, cf_cv_dcl_$1,[
	AC_TRY_COMPILE([
$ac_includes_default
#include <errno.h> ],
	ifelse([$2],,int,[$2]) x = (ifelse([$2],,int,[$2])) $1; (void)x,
	[cf_cv_dcl_$1=yes],
	[cf_cv_dcl_$1=no])
])

if test "$cf_cv_dcl_$1" = no ; then
	CF_UPPER(cf_result,decl_$1)
	AC_DEFINE_UNQUOTED($cf_result)
fi

# It's possible (for near-UNIX clones) that the data doesn't exist
CF_CHECK_EXTERN_DATA($1,ifelse([$2],,int,[$2]))
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_EXTERN_DATA version: 5 updated: 2021/09/04 06:35:04
dnl --------------------
dnl Check for existence of external data in the current set of libraries.  If
dnl we can modify it, it is real enough.
dnl $1 = the name to check
dnl $2 = its type
AC_DEFUN([CF_CHECK_EXTERN_DATA],
[
AC_CACHE_CHECK(if external $1 exists, cf_cv_have_$1,[
	AC_TRY_LINK([
#undef $1
extern $2 $1;
],
	[$1 = 2],
	[cf_cv_have_$1=yes],
	[cf_cv_have_$1=no])
])

if test "$cf_cv_have_$1" = yes ; then
	CF_UPPER(cf_result,have_$1)
	AC_DEFINE_UNQUOTED($cf_result)
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_TYPE version: 4 updated: 2021/01/02 09:31:20
dnl -------------
dnl Add a 3rd parameter to AC_CHECK_TYPE, working around autoconf 2.5x's
dnl deliberate incompatibility.
dnl	$1 = name of type to check for
dnl	$2 = default type
dnl	$3 = additional #include's and related preprocessor lines.
ifdef([m4_HAS_AC_CT_FOURARGS], [m4_undefine([m4_HAS_AC_CT_FOURARGS])])dnl
ifelse(m4_PACKAGE_VERSION, [fnord_acsalt], [],
[ifdef([m4_version_compare],[m4_define([m4_HAS_AC_CT_FOURARGS])])])dnl
AC_DEFUN([CF_CHECK_TYPE],
[
ifdef([m4_HAS_AC_CT_FOURARGS],[
	AC_CHECK_TYPE([$1],ac_cv_type_$1=yes,ac_cv_type_$1=no,[$3])
	],[
	AC_MSG_CHECKING(for $1)
	AC_TRY_COMPILE([
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
$3
],[
	static $1 dummy; if (sizeof(dummy)) return 0; else return 1;],
	ac_cv_type_$1=yes,
	ac_cv_type_$1=no)
	AC_MSG_RESULT($ac_cv_type_$1)
])dnl
if test "$ac_cv_type_$1" = no; then
	AC_DEFINE($1, $2, Define to $2 if $1 is not declared)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CLANG_COMPILER version: 9 updated: 2023/02/18 17:41:25
dnl -----------------
dnl Check if the given compiler is really clang.  clang's C driver defines
dnl __GNUC__ (fooling the configure script into setting $GCC to yes) but does
dnl not ignore some gcc options.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = CLANG_COMPILER (default)
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_CLANG_COMPILER],[
ifelse([$2],,CLANG_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	AC_MSG_CHECKING(if this is really Clang ifelse([$1],GXX,C++,C) compiler)
	cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
	AC_TRY_COMPILE([],[
#ifdef __clang__
#else
#error __clang__ is not defined
#endif
],[ifelse([$2],,CLANG_COMPILER,[$2])=yes
],[])
	ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
	AC_MSG_RESULT($ifelse([$2],,CLANG_COMPILER,[$2]))
fi

CLANG_VERSION=none

if test "x$ifelse([$2],,CLANG_COMPILER,[$2])" = "xyes" ; then
	case "$CC" in
	(c[[1-9]][[0-9]]|*/c[[1-9]][[0-9]])
		AC_MSG_WARN(replacing broken compiler alias $CC)
		CFLAGS="$CFLAGS -std=`echo "$CC" | sed -e 's%.*/%%'`"
		CC=clang
		;;
	esac

	AC_MSG_CHECKING(version of $CC)
	CLANG_VERSION="`$CC --version 2>/dev/null | sed -e '2,$d' -e 's/^.*(CLANG[[^)]]*) //' -e 's/^.*(Debian[[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$CLANG_VERSION" && CLANG_VERSION=unknown
	AC_MSG_RESULT($CLANG_VERSION)

	for cf_clang_opt in \
		-Qunused-arguments \
		-Wno-error=implicit-function-declaration
	do
		AC_MSG_CHECKING(if option $cf_clang_opt works)
		cf_save_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $cf_clang_opt"
		AC_TRY_LINK([
			#include <stdio.h>],[
			printf("hello!\\n");],[
			cf_clang_optok=yes],[
			cf_clang_optok=no])
		AC_MSG_RESULT($cf_clang_optok)
		CFLAGS="$cf_save_CFLAGS"
		if test "$cf_clang_optok" = yes; then
			CF_VERBOSE(adding option $cf_clang_opt)
			CF_APPEND_TEXT(CFLAGS,$cf_clang_opt)
		fi
	done
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 14 updated: 2021/09/04 06:35:04
dnl ---------------
dnl You can always use "make -n" to see the actual options, but it is hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LT - symbol to control if libtool is verbose
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          do not display "compiling" commands],
	[
	ECHO_LT='--silent'
	ECHO_LD='@echo linking [$]@;'
	RULE_CC='@echo compiling [$]<'
	SHOW_CC='@echo compiling [$]@'
	ECHO_CC='@'
],[
	ECHO_LT=''
	ECHO_LD=''
	RULE_CC=''
	SHOW_CC=''
	ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LT)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_LEAKS version: 9 updated: 2021/04/03 16:41:50
dnl ----------------
dnl Combine no-leak checks with the libraries or tools that are used for the
dnl checks.
AC_DEFUN([CF_DISABLE_LEAKS],[

AC_REQUIRE([CF_WITH_DMALLOC])
AC_REQUIRE([CF_WITH_DBMALLOC])
AC_REQUIRE([CF_WITH_VALGRIND])

AC_MSG_CHECKING(if you want to perform memory-leak testing)
AC_ARG_ENABLE(leaks,
	[  --disable-leaks         test: free permanent memory, analyze leaks],
	[enable_leaks=$enableval],
	[enable_leaks=yes])
dnl with_no_leaks is more readable...
if test "x$enable_leaks" = xno; then with_no_leaks=yes; else with_no_leaks=no; fi
AC_MSG_RESULT($with_no_leaks)

if test "$enable_leaks" = no ; then
	AC_DEFINE(NO_LEAKS,1,[Define to 1 if you want to perform memory-leak testing.])
	AC_DEFINE(YY_NO_LEAKS,1,[Define to 1 if you want to perform memory-leak testing.])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_RPATH_HACK version: 3 updated: 2021/01/05 20:14:44
dnl ---------------------
dnl The rpath-hack makes it simpler to build programs, particularly with the
dnl *BSD ports which may have essential libraries in unusual places.  But it
dnl can interfere with building an executable for the base system.  Use this
dnl option in that case.
AC_DEFUN([CF_DISABLE_RPATH_HACK],
[
AC_MSG_CHECKING(if rpath-hack should be disabled)
CF_ARG_DISABLE(rpath-hack,
	[  --disable-rpath-hack    don't add rpath options for additional libraries],
	[enable_rpath_hack=no],
	[enable_rpath_hack=yes])
dnl TODO - drop cf_disable_rpath_hack
if test "x$enable_rpath_hack" = xno; then cf_disable_rpath_hack=yes; else cf_disable_rpath_hack=no; fi
AC_MSG_RESULT($cf_disable_rpath_hack)

if test "$enable_rpath_hack" = yes ; then
	CF_RPATH_HACK
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_ENABLE_TRACE version: 3 updated: 2012/10/04 05:24:07
dnl ---------------
AC_DEFUN([CF_ENABLE_TRACE],[
AC_MSG_CHECKING(if you want to enable debugging trace)
CF_ARG_ENABLE(trace,
	[  --enable-trace          test: turn on debug-tracing],
	[with_trace=yes],
	[with_trace=no])
AC_MSG_RESULT($with_trace)
if test "$with_trace" = "yes"
then
	AC_DEFINE(OPT_TRACE,1,[Define to 1 if you want to enable debugging trace])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ENABLE_WARNINGS version: 9 updated: 2021/01/05 19:40:50
dnl ------------------
dnl Configure-option to enable gcc warnings
dnl
dnl $1 = extra options to add, if supported
dnl $2 = option for checking attributes.  By default, this is done when
dnl      warnings are enabled.  For other values:
dnl      yes: always do this, e.g., to use in generated library-headers
dnl      no: never do this
AC_DEFUN([CF_ENABLE_WARNINGS],[
if test "$GCC" = yes || test "$GXX" = yes
then
CF_FIX_WARNINGS(CFLAGS)
CF_FIX_WARNINGS(CPPFLAGS)
CF_FIX_WARNINGS(LDFLAGS)
AC_MSG_CHECKING(if you want to turn on gcc warnings)
CF_ARG_ENABLE(warnings,
	[  --enable-warnings       test: turn on gcc compiler warnings],
	[enable_warnings=yes],
	[enable_warnings=no])
AC_MSG_RESULT($enable_warnings)
if test "$enable_warnings" = "yes"
then
	ifelse($2,,[CF_GCC_ATTRIBUTES])
	CF_GCC_WARNINGS($1)
fi
ifelse($2,yes,[CF_GCC_ATTRIBUTES])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_LINKAGE version: 22 updated: 2020/12/31 20:19:42
dnl ---------------
dnl Find a library (specifically the linkage used in the code fragment),
dnl searching for it if it is not already in the library path.
dnl See also CF_ADD_SEARCHPATH.
dnl
dnl Parameters (4-on are optional):
dnl     $1 = headers for library entrypoint
dnl     $2 = code fragment for library entrypoint
dnl     $3 = the library name without the "-l" option or ".so" suffix.
dnl     $4 = action to perform if successful (default: update CPPFLAGS, etc)
dnl     $5 = action to perform if not successful
dnl     $6 = module name, if not the same as the library name
dnl     $7 = extra libraries
dnl
dnl Sets these variables:
dnl     $cf_cv_find_linkage_$3 - yes/no according to whether linkage is found
dnl     $cf_cv_header_path_$3 - include-directory if needed
dnl     $cf_cv_library_path_$3 - library-directory if needed
dnl     $cf_cv_library_file_$3 - library-file if needed, e.g., -l$3
AC_DEFUN([CF_FIND_LINKAGE],[

# If the linkage is not already in the $CPPFLAGS/$LDFLAGS configuration, these
# will be set on completion of the AC_TRY_LINK below.
cf_cv_header_path_$3=
cf_cv_library_path_$3=

CF_MSG_LOG([Starting [FIND_LINKAGE]($3,$6)])

cf_save_LIBS="$LIBS"

AC_TRY_LINK([$1],[$2],[
	cf_cv_find_linkage_$3=yes
	cf_cv_header_path_$3=/usr/include
	cf_cv_library_path_$3=/usr/lib
],[

LIBS="-l$3 $7 $cf_save_LIBS"

AC_TRY_LINK([$1],[$2],[
	cf_cv_find_linkage_$3=yes
	cf_cv_header_path_$3=/usr/include
	cf_cv_library_path_$3=/usr/lib
	cf_cv_library_file_$3="-l$3"
],[
	cf_cv_find_linkage_$3=no
	LIBS="$cf_save_LIBS"

	CF_VERBOSE(find linkage for $3 library)
	CF_MSG_LOG([Searching for headers in [FIND_LINKAGE]($3,$6)])

	cf_save_CPPFLAGS="$CPPFLAGS"
	cf_test_CPPFLAGS="$CPPFLAGS"

	CF_HEADER_PATH(cf_search,ifelse([$6],,[$3],[$6]))
	for cf_cv_header_path_$3 in $cf_search
	do
		if test -d "$cf_cv_header_path_$3" ; then
			CF_VERBOSE(... testing $cf_cv_header_path_$3)
			CPPFLAGS="$cf_save_CPPFLAGS"
			CF_APPEND_TEXT(CPPFLAGS,-I$cf_cv_header_path_$3)
			AC_TRY_COMPILE([$1],[$2],[
				CF_VERBOSE(... found $3 headers in $cf_cv_header_path_$3)
				cf_cv_find_linkage_$3=maybe
				cf_test_CPPFLAGS="$CPPFLAGS"
				break],[
				CPPFLAGS="$cf_save_CPPFLAGS"
				])
		fi
	done

	if test "$cf_cv_find_linkage_$3" = maybe ; then

		CF_MSG_LOG([Searching for $3 library in [FIND_LINKAGE]($3,$6)])

		cf_save_LIBS="$LIBS"
		cf_save_LDFLAGS="$LDFLAGS"

		ifelse([$6],,,[
		CPPFLAGS="$cf_test_CPPFLAGS"
		LIBS="-l$3 $7 $cf_save_LIBS"
		AC_TRY_LINK([$1],[$2],[
			CF_VERBOSE(... found $3 library in system)
			cf_cv_find_linkage_$3=yes])
			CPPFLAGS="$cf_save_CPPFLAGS"
			LIBS="$cf_save_LIBS"
			])

		if test "$cf_cv_find_linkage_$3" != yes ; then
			CF_LIBRARY_PATH(cf_search,$3)
			for cf_cv_library_path_$3 in $cf_search
			do
				if test -d "$cf_cv_library_path_$3" ; then
					CF_VERBOSE(... testing $cf_cv_library_path_$3)
					CPPFLAGS="$cf_test_CPPFLAGS"
					LIBS="-l$3 $7 $cf_save_LIBS"
					LDFLAGS="$cf_save_LDFLAGS -L$cf_cv_library_path_$3"
					AC_TRY_LINK([$1],[$2],[
					CF_VERBOSE(... found $3 library in $cf_cv_library_path_$3)
					cf_cv_find_linkage_$3=yes
					cf_cv_library_file_$3="-l$3"
					break],[
					CPPFLAGS="$cf_save_CPPFLAGS"
					LIBS="$cf_save_LIBS"
					LDFLAGS="$cf_save_LDFLAGS"
					])
				fi
			done
			CPPFLAGS="$cf_save_CPPFLAGS"
			LDFLAGS="$cf_save_LDFLAGS"
		fi

	else
		cf_cv_find_linkage_$3=no
	fi
	],$7)
])

LIBS="$cf_save_LIBS"

if test "$cf_cv_find_linkage_$3" = yes ; then
ifelse([$4],,[
	CF_ADD_INCDIR($cf_cv_header_path_$3)
	CF_ADD_LIBDIR($cf_cv_library_path_$3)
	CF_ADD_LIB($3)
],[$4])
else
ifelse([$5],,AC_MSG_WARN(Cannot find $3 library),[$5])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIX_WARNINGS version: 4 updated: 2021/12/16 18:22:31
dnl ---------------
dnl Warning flags do not belong in CFLAGS, CPPFLAGS, etc.  Any of gcc's
dnl "-Werror" flags can interfere with configure-checks.  Those go into
dnl EXTRA_CFLAGS.
dnl
dnl $1 = variable name to repair
define([CF_FIX_WARNINGS],[
if test "$GCC" = yes || test "$GXX" = yes
then
	case [$]$1 in
	(*-Werror=*)
		cf_temp_flags=
		for cf_temp_scan in [$]$1
		do
			case "x$cf_temp_scan" in
			(x-Werror=format*)
				CF_APPEND_TEXT(cf_temp_flags,$cf_temp_scan)
				;;
			(x-Werror=*)
				CF_APPEND_TEXT(EXTRA_CFLAGS,$cf_temp_scan)
				;;
			(*)
				CF_APPEND_TEXT(cf_temp_flags,$cf_temp_scan)
				;;
			esac
		done
		if test "x[$]$1" != "x$cf_temp_flags"
		then
			CF_VERBOSE(repairing $1: [$]$1)
			$1="$cf_temp_flags"
			CF_VERBOSE(... fixed [$]$1)
			CF_VERBOSE(... extra $EXTRA_CFLAGS)
		fi
		;;
	esac
fi
AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_GRANTPT version: 15 updated: 2020/12/31 18:40:20
dnl ---------------
dnl Check for grantpt versus openpty, as well as functions that "should" be
dnl available if grantpt is available.
AC_DEFUN([CF_FUNC_GRANTPT],[

AC_CHECK_HEADERS( \
stropts.h \
)

cf_func_grantpt="grantpt ptsname"
cf_prefer_openpt=no
case $host_os in
(darwin[[0-9]].*)
	;;
(openbsd[[0-9]].*)
	# The POSIX entrypoints exist, but have never worked.
	;;
(linux*)
	cf_func_grantpt="$cf_func_grantpt posix_openpt"
	cf_prefer_openpt=yes
	;;
(*)
	cf_func_grantpt="$cf_func_grantpt posix_openpt"
	;;
esac

AC_CHECK_FUNCS($cf_func_grantpt)

cf_grantpt_opts=
if test "x$ac_cv_func_grantpt" = "xyes" ; then
	AC_MSG_CHECKING(if grantpt really works)
	AC_TRY_LINK(CF__GRANTPT_HEAD,CF__GRANTPT_BODY,[
	AC_TRY_RUN(CF__GRANTPT_HEAD
int main(void)
{
CF__GRANTPT_BODY
}
,
,ac_cv_func_grantpt=no
,ac_cv_func_grantpt=maybe)
	],ac_cv_func_grantpt=no)
	AC_MSG_RESULT($ac_cv_func_grantpt)

	if test "x$ac_cv_func_grantpt" != "xno" ; then

		if test "x$ac_cv_func_grantpt" = "xyes" ; then
			AC_MSG_CHECKING(for pty features)
dnl if we have no stropts.h, skip the checks for streams modules
			if test "x$ac_cv_header_stropts_h" = xyes
			then
				cf_pty_this=0
			else
				cf_pty_this=3
			fi

			cf_pty_defines=
			while test $cf_pty_this != 6
			do

				cf_pty_feature=
				cf_pty_next="`expr $cf_pty_this + 1`"
				CF_MSG_LOG(pty feature test $cf_pty_next:5)
				AC_TRY_RUN(#define CONFTEST $cf_pty_this
$cf_pty_defines
CF__GRANTPT_HEAD
int main(void)
{
CF__GRANTPT_BODY
}
,
[
				case $cf_pty_next in
				(1) # - streams
					cf_pty_feature=ptem
					;;
				(2) # - streams
					cf_pty_feature=ldterm
					;;
				(3) # - streams
					cf_pty_feature=ttcompat
					;;
				(4)
					cf_pty_feature=pty_isatty
					;;
				(5)
					cf_pty_feature=pty_tcsetattr
					;;
				(6)
					cf_pty_feature=tty_tcsetattr
					;;
				esac
],[
				case $cf_pty_next in
				(1|2|3)
					CF_MSG_LOG(skipping remaining streams features $cf_pty_this..2)
					cf_pty_next=3
					;;
				esac
])
				if test -n "$cf_pty_feature"
				then
					cf_pty_defines="$cf_pty_defines
#define CONFTEST_$cf_pty_feature 1
"
					cf_grantpt_opts="$cf_grantpt_opts $cf_pty_feature"
				fi

				cf_pty_this=$cf_pty_next
			done
			AC_MSG_RESULT($cf_grantpt_opts)
			cf_grantpt_opts=`echo "$cf_grantpt_opts" | sed -e 's/ isatty//'`
		fi
	fi
fi

dnl If we found grantpt, but no features, e.g., for streams or if we are not
dnl able to use tcsetattr, then give openpty a try.  In particular, Darwin 10.7
dnl has a more functional openpty than posix_openpt.
dnl
dnl There is no configure run-test for openpty, since older implementations do
dnl not always run properly as a non-root user.  For that reason, we also allow
dnl the configure script to suppress this check entirely with $disable_openpty.
if test "x$cf_prefer_posix_openpt" = "xyes" && test "x$ac_cv_func_posix_openpt" = "xyes" ; then
	CF_VERBOSE(prefer posix_openpt over openpty)
elif test "x$disable_openpty" != "xyes" || test -z "$cf_grantpt_opts" ; then
	AC_CHECK_LIB(util, openpty, [cf_have_openpty=yes],[cf_have_openpty=no])
	if test "$cf_have_openpty" = yes ; then
		ac_cv_func_grantpt=no
		LIBS="-lutil $LIBS"
		AC_DEFINE(HAVE_OPENPTY,1,[Define to 1 if you have the openpty function])
		AC_CHECK_HEADERS( \
			util.h \
			libutil.h \
			pty.h \
		)
	fi
fi

dnl If we did not settle on using openpty, fill in the definitions for grantpt.
if test "x$ac_cv_func_grantpt" != xno
then
	CF_VERBOSE(will rely upon grantpt)
	AC_DEFINE(HAVE_WORKING_GRANTPT,1,[Define to 1 if the grantpt function seems to work])
	for cf_feature in $cf_grantpt_opts
	do
		cf_feature=`echo "$cf_feature" | sed -e 's/ //g'`
		CF_UPPER(cf_FEATURE,$cf_feature)
		AC_DEFINE_UNQUOTED(HAVE_GRANTPT_$cf_FEATURE)
	done
elif test "x$cf_have_openpty" = xno
then
	CF_VERBOSE(will rely upon BSD-pseudoterminals)
else
	CF_VERBOSE(will rely upon openpty)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_POLL version: 11 updated: 2023/01/05 18:51:40
dnl ------------
dnl See if the poll function really works.  Some platforms have poll(), but
dnl it does not work for terminals or files.
AC_DEFUN([CF_FUNC_POLL],[
tty >/dev/null 2>&1 || { AC_CHECK_FUNCS(posix_openpt) }
AC_CACHE_CHECK(if poll really works,cf_cv_working_poll,[
AC_TRY_RUN([
$ac_includes_default

#include <fcntl.h>

#ifdef HAVE_POLL_H
#include <poll.h>
#else
#include <sys/poll.h>
#endif

int main(void) {
	struct pollfd myfds;
	int ret;

	/* check for Darwin bug with respect to "devices" */
	myfds.fd = open("/dev/null", 1);	/* O_WRONLY */
	if (myfds.fd < 0)
		myfds.fd = 0;
	myfds.events = POLLIN;
	myfds.revents = 0;

	ret = poll(&myfds, 1, 100);

	if (ret < 0 || (myfds.revents & POLLNVAL)) {
		ret = -1;
	} else {
		int fd = 0;
		if (!isatty(fd)) {
			fd = open("/dev/tty", 2);	/* O_RDWR */
		}
#ifdef HAVE_POSIX_OPENPT
		if (fd < 0) {
			fd = posix_openpt(O_RDWR);
		}
#endif

		if (fd >= 0) {
			/* also check with standard input */
			myfds.fd = fd;
			myfds.events = POLLIN;
			myfds.revents = 0;
			ret = poll(&myfds, 1, 100);
		} else {
			ret = -1;
		}
	}
	${cf_cv_main_return:-return}(ret < 0);
}],
	[cf_cv_working_poll=yes],
	[cf_cv_working_poll=no],
	[cf_cv_working_poll=unknown])])
test "$cf_cv_working_poll" = "yes" && AC_DEFINE(HAVE_WORKING_POLL,1,[Define to 1 if the poll function seems to work])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_SETGROUPS version: 1 updated: 2024/01/02 19:01:22
dnl -----------------
dnl Check for getgroups() and setgroups() functions
AC_DEFUN([CF_FUNC_SETGROUPS],[
AC_CACHE_CHECK(for getgroups/setgroups,cf_cv_func_setgroups,[
AC_TRY_LINK([
$ac_includes_default
#include <grp.h>
],[
gid_t my_list[10];
if (getgroups(0, my_list)) setgroups(0, NULL);
],[cf_cv_func_setgroups=yes],[cf_cv_func_setgroups=no])
])
test "$cf_cv_func_setgroups" = yes && AC_DEFINE(HAVE_SETGROUPS,1,[Define to 1 if setgroups function exists])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_ATTRIBUTES version: 24 updated: 2021/03/20 12:00:25
dnl -----------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.  Though useful, not all are supported -- and contrary
dnl to documentation, unrecognized directives cause older compilers to barf.
AC_DEFUN([CF_GCC_ATTRIBUTES],
[AC_REQUIRE([AC_PROG_FGREP])dnl
AC_REQUIRE([CF_C11_NORETURN])dnl

if test "$GCC" = yes || test "$GXX" = yes
then
cat > conftest.i <<EOF
#ifndef GCC_PRINTF
#define GCC_PRINTF 0
#endif
#ifndef GCC_SCANF
#define GCC_SCANF 0
#endif
#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
EOF
if test "$GCC" = yes
then
	AC_CHECKING([for $CC __attribute__ directives])
cat > "conftest.$ac_ext" <<EOF
#line __oline__ "${as_me:-configure}"
#include <stdio.h>
#include "confdefs.h"
#include "conftest.h"
#include "conftest.i"
#if	GCC_PRINTF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var) /*nothing*/
#endif
#if	GCC_SCANF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)  /*nothing*/
#endif
extern void wow(char *,...) GCC_SCANFLIKE(1,2);
extern GCC_NORETURN void oops(char *,...) GCC_PRINTFLIKE(1,2);
extern GCC_NORETURN void foo(void);
int main(int argc GCC_UNUSED, char *argv[[]] GCC_UNUSED) { (void)argc; (void)argv; return 0; }
EOF
	cf_printf_attribute=no
	cf_scanf_attribute=no
	for cf_attribute in scanf printf unused noreturn
	do
		CF_UPPER(cf_ATTRIBUTE,$cf_attribute)
		cf_directive="__attribute__(($cf_attribute))"
		echo "checking for $CC $cf_directive" 1>&AC_FD_CC

		case "$cf_attribute" in
		(printf)
			cf_printf_attribute=yes
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		(scanf)
			cf_scanf_attribute=yes
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		(*)
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE $cf_directive
EOF
			;;
		esac

		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... $cf_attribute)
			cat conftest.h >>confdefs.h
			case "$cf_attribute" in
			(noreturn)
				AC_DEFINE_UNQUOTED(GCC_NORETURN,$cf_directive,[Define to noreturn-attribute for gcc])
				;;
			(printf)
				cf_value='/* nothing */'
				if test "$cf_printf_attribute" != no ; then
					cf_value='__attribute__((format(printf,fmt,var)))'
					AC_DEFINE(GCC_PRINTF,1,[Define to 1 if the compiler supports gcc-like printf attribute.])
				fi
				AC_DEFINE_UNQUOTED(GCC_PRINTFLIKE(fmt,var),$cf_value,[Define to printf-attribute for gcc])
				;;
			(scanf)
				cf_value='/* nothing */'
				if test "$cf_scanf_attribute" != no ; then
					cf_value='__attribute__((format(scanf,fmt,var)))'
					AC_DEFINE(GCC_SCANF,1,[Define to 1 if the compiler supports gcc-like scanf attribute.])
				fi
				AC_DEFINE_UNQUOTED(GCC_SCANFLIKE(fmt,var),$cf_value,[Define to sscanf-attribute for gcc])
				;;
			(unused)
				AC_DEFINE_UNQUOTED(GCC_UNUSED,$cf_directive,[Define to unused-attribute for gcc])
				;;
			esac
		fi
	done
else
	${FGREP-fgrep} define conftest.i >>confdefs.h
fi
rm -rf ./conftest*
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 9 updated: 2023/03/05 14:30:13
dnl --------------
dnl Find version of gcc, and (because icc/clang pretend to be gcc without being
dnl compatible), attempt to determine if icc/clang is actually used.
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version 2>/dev/null | sed -e '2,$d' -e 's/^[[^(]]*([[^)]][[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
CF_INTEL_COMPILER(GCC,INTEL_COMPILER,CFLAGS)
CF_CLANG_COMPILER(GCC,CLANG_COMPILER,CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 43 updated: 2024/12/21 08:44:12
dnl ---------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Winline (usually not worthwhile)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally).  This
dnl		is enabled for ncurses using "--enable-const".
dnl	-pedantic
dnl
dnl Parameter:
dnl	$1 is an optional list of gcc warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
AC_REQUIRE([CF_GCC_VERSION])
if test "x$have_x" = xyes; then CF_CONST_X_STRING fi
cat > "conftest.$ac_ext" <<EOF
#line __oline__ "${as_me:-configure}"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF
if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #279: controlling expression is constant

	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall"
	for cf_opt in \
		wd1419 \
		wd1683 \
		wd1684 \
		wd193 \
		wd593 \
		wd279 \
		wd810 \
		wd869 \
		wd981
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
elif test "$GCC" = yes && test "$GCC_VERSION" != "unknown"
then
	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	cf_gcc_warnings="Wignored-qualifiers Wlogical-op Wvarargs"
	test "x$CLANG_COMPILER" = xyes && cf_gcc_warnings=
	for cf_opt in W Wall \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Wdeclaration-after-statement \
		Wextra \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef Wno-inline $cf_gcc_warnings $cf_warn_CONST $1
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			case "$cf_opt" in
			(Winline)
				case "$GCC_VERSION" in
				([[34]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			(Wpointer-arith)
				case "$GCC_VERSION" in
				([[12]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			esac
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
fi
rm -rf ./conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GLOB_FULLPATH version: 2 updated: 2024/08/03 12:34:02
dnl ----------------
dnl Use this in case-statements to check for pathname syntax, i.e., absolute
dnl pathnames.  The "x" is assumed since we provide an alternate form for DOS.
AC_DEFUN([CF_GLOB_FULLPATH],[
AC_REQUIRE([CF_WITH_SYSTYPE])dnl
case "$cf_cv_system_name" in
(cygwin*|msys*|mingw32*|mingw64|os2*)
	GLOB_FULLPATH_POSIX='/*'
	GLOB_FULLPATH_OTHER='[[a-zA-Z]]:[[\\/]]*'
	;;
(*)
	GLOB_FULLPATH_POSIX='/*'
	GLOB_FULLPATH_OTHER=$GLOB_FULLPATH_POSIX
	;;
esac
AC_SUBST(GLOB_FULLPATH_POSIX)
AC_SUBST(GLOB_FULLPATH_OTHER)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNU_SOURCE version: 10 updated: 2018/12/10 20:09:41
dnl -------------
dnl Check if we must define _GNU_SOURCE to get a reasonable value for
dnl _XOPEN_SOURCE, upon which many POSIX definitions depend.  This is a defect
dnl (or misfeature) of glibc2, which breaks portability of many applications,
dnl since it is interwoven with GNU extensions.
dnl
dnl Well, yes we could work around it...
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
AC_DEFUN([CF_GNU_SOURCE],
[
cf_gnu_xopen_source=ifelse($1,,500,$1)

AC_CACHE_CHECK(if this is the GNU C library,cf_cv_gnu_library,[
AC_TRY_COMPILE([#include <sys/types.h>],[
	#if __GLIBC__ > 0 && __GLIBC_MINOR__ >= 0
		return 0;
	#elif __NEWLIB__ > 0 && __NEWLIB_MINOR__ >= 0
		return 0;
	#else
	#	error not GNU C library
	#endif],
	[cf_cv_gnu_library=yes],
	[cf_cv_gnu_library=no])
])

if test x$cf_cv_gnu_library = xyes; then

	# With glibc 2.19 (13 years after this check was begun), _DEFAULT_SOURCE
	# was changed to help a little.  newlib incorporated the change about 4
	# years later.
	AC_CACHE_CHECK(if _DEFAULT_SOURCE can be used as a basis,cf_cv_gnu_library_219,[
		cf_save="$CPPFLAGS"
		CF_APPEND_TEXT(CPPFLAGS,-D_DEFAULT_SOURCE)
		AC_TRY_COMPILE([#include <sys/types.h>],[
			#if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 19) || (__GLIBC__ > 2)
				return 0;
			#elif (__NEWLIB__ == 2 && __NEWLIB_MINOR__ >= 4) || (__GLIBC__ > 3)
				return 0;
			#else
			#	error GNU C library __GLIBC__.__GLIBC_MINOR__ is too old
			#endif],
			[cf_cv_gnu_library_219=yes],
			[cf_cv_gnu_library_219=no])
		CPPFLAGS="$cf_save"
	])

	if test "x$cf_cv_gnu_library_219" = xyes; then
		cf_save="$CPPFLAGS"
		AC_CACHE_CHECK(if _XOPEN_SOURCE=$cf_gnu_xopen_source works with _DEFAULT_SOURCE,cf_cv_gnu_dftsrc_219,[
			CF_ADD_CFLAGS(-D_DEFAULT_SOURCE -D_XOPEN_SOURCE=$cf_gnu_xopen_source)
			AC_TRY_COMPILE([
				#include <limits.h>
				#include <sys/types.h>
				],[
				#if (_XOPEN_SOURCE >= $cf_gnu_xopen_source) && (MB_LEN_MAX > 1)
					return 0;
				#else
				#	error GNU C library is too old
				#endif],
				[cf_cv_gnu_dftsrc_219=yes],
				[cf_cv_gnu_dftsrc_219=no])
			])
		test "x$cf_cv_gnu_dftsrc_219" = "xyes" || CPPFLAGS="$cf_save"
	else
		cf_cv_gnu_dftsrc_219=maybe
	fi

	if test "x$cf_cv_gnu_dftsrc_219" != xyes; then

		AC_CACHE_CHECK(if we must define _GNU_SOURCE,cf_cv_gnu_source,[
		AC_TRY_COMPILE([#include <sys/types.h>],[
			#ifndef _XOPEN_SOURCE
			#error	expected _XOPEN_SOURCE to be defined
			#endif],
			[cf_cv_gnu_source=no],
			[cf_save="$CPPFLAGS"
			 CF_ADD_CFLAGS(-D_GNU_SOURCE)
			 AC_TRY_COMPILE([#include <sys/types.h>],[
				#ifdef _XOPEN_SOURCE
				#error	expected _XOPEN_SOURCE to be undefined
				#endif],
				[cf_cv_gnu_source=no],
				[cf_cv_gnu_source=yes])
			CPPFLAGS="$cf_save"
			])
		])

		if test "$cf_cv_gnu_source" = yes
		then
		AC_CACHE_CHECK(if we should also define _DEFAULT_SOURCE,cf_cv_default_source,[
			CF_APPEND_TEXT(CPPFLAGS,-D_GNU_SOURCE)
			AC_TRY_COMPILE([#include <sys/types.h>],[
				#ifdef _DEFAULT_SOURCE
				#error	expected _DEFAULT_SOURCE to be undefined
				#endif],
				[cf_cv_default_source=no],
				[cf_cv_default_source=yes])
			])
			if test "$cf_cv_default_source" = yes
			then
				CF_APPEND_TEXT(CPPFLAGS,-D_DEFAULT_SOURCE)
			fi
		fi
	fi

fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HEADER_PATH version: 15 updated: 2021/01/01 13:31:04
dnl --------------
dnl Construct a search-list of directories for a nonstandard header-file
dnl
dnl Parameters
dnl	$1 = the variable to return as result
dnl	$2 = the package name
AC_DEFUN([CF_HEADER_PATH],
[
$1=

# collect the current set of include-directories from compiler flags
cf_header_path_list=""
if test -n "${CFLAGS}${CPPFLAGS}" ; then
	for cf_header_path in $CPPFLAGS $CFLAGS
	do
		case "$cf_header_path" in
		(-I*)
			cf_header_path=`echo ".$cf_header_path" |sed -e 's/^...//' -e 's,/include$,,'`
			CF_ADD_SUBDIR_PATH($1,$2,include,$cf_header_path,NONE)
			cf_header_path_list="$cf_header_path_list [$]$1"
			;;
		esac
	done
fi

# add the variations for the package we are looking for
CF_SUBDIR_PATH($1,$2,include)

test "$includedir" != NONE && \
test "$includedir" != "/usr/include" && \
test -d "$includedir" && {
	test -d "$includedir" &&    $1="[$]$1 $includedir"
	test -d "$includedir/$2" && $1="[$]$1 $includedir/$2"
}

test "$oldincludedir" != NONE && \
test "$oldincludedir" != "/usr/include" && \
test -d "$oldincludedir" && {
	test -d "$oldincludedir"    && $1="[$]$1 $oldincludedir"
	test -d "$oldincludedir/$2" && $1="[$]$1 $oldincludedir/$2"
}

$1="[$]$1 $cf_header_path_list"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 9 updated: 2023/02/18 17:41:25
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.  It
dnl tries to imitate gcc, but does not return an error when it finds a mismatch
dnl between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_INTEL_COMPILER],[
AC_REQUIRE([AC_CANONICAL_HOST])
ifelse([$2],,INTEL_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	case "$host_os" in
	(linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse([$1],GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
		ifelse([$3],,CFLAGS,[$3])="$ifelse([$3],,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
#error __INTEL_COMPILER is not defined
#endif
],[ifelse([$2],,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147"
],[])
		ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse([$2],,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LD_RPATH_OPT version: 9 updated: 2021/01/01 13:31:04
dnl ---------------
dnl For the given system and compiler, find the compiler flags to pass to the
dnl loader to use the "rpath" feature.
AC_DEFUN([CF_LD_RPATH_OPT],
[
AC_REQUIRE([CF_CHECK_CACHE])

LD_RPATH_OPT=
if test "x$cf_cv_enable_rpath" != xno
then
	AC_MSG_CHECKING(for an rpath option)
	case "$cf_cv_system_name" in
	(irix*)
		if test "$GCC" = yes; then
			LD_RPATH_OPT="-Wl,-rpath,"
		else
			LD_RPATH_OPT="-rpath "
		fi
		;;
	(linux*|gnu*|k*bsd*-gnu|freebsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(openbsd[[2-9]].*|mirbsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(dragonfly*)
		LD_RPATH_OPT="-rpath "
		;;
	(netbsd*)
		LD_RPATH_OPT="-Wl,-rpath,"
		;;
	(osf*|mls+*)
		LD_RPATH_OPT="-rpath "
		;;
	(solaris2*)
		LD_RPATH_OPT="-R"
		;;
	(*)
		;;
	esac
	AC_MSG_RESULT($LD_RPATH_OPT)

	case "x$LD_RPATH_OPT" in
	(x-R*)
		AC_MSG_CHECKING(if we need a space after rpath option)
		cf_save_LIBS="$LIBS"
		CF_ADD_LIBS(${LD_RPATH_OPT}$libdir)
		AC_TRY_LINK(, , cf_rpath_space=no, cf_rpath_space=yes)
		LIBS="$cf_save_LIBS"
		AC_MSG_RESULT($cf_rpath_space)
		test "$cf_rpath_space" = yes && LD_RPATH_OPT="$LD_RPATH_OPT "
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIBRARY_PATH version: 11 updated: 2021/01/01 13:31:04
dnl ---------------
dnl Construct a search-list of directories for a nonstandard library-file
dnl
dnl Parameters
dnl	$1 = the variable to return as result
dnl	$2 = the package name
AC_DEFUN([CF_LIBRARY_PATH],
[
$1=
cf_library_path_list=""
if test -n "${LDFLAGS}${LIBS}" ; then
	for cf_library_path in $LDFLAGS $LIBS
	do
		case "$cf_library_path" in
		(-L*)
			cf_library_path=`echo ".$cf_library_path" |sed -e 's/^...//' -e 's,/lib$,,'`
			CF_ADD_SUBDIR_PATH($1,$2,lib,$cf_library_path,NONE)
			cf_library_path_list="$cf_library_path_list [$]$1"
			;;
		esac
	done
fi

CF_SUBDIR_PATH($1,$2,lib)

$1="$cf_library_path_list [$]$1"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKE_DOCS version: 5 updated: 2021/01/10 16:05:11
dnl ------------
dnl $1 = name(s) to generate rules for
dnl $2 = suffix of corresponding manpages used as input.
dnl
dnl This works best if called at the end of configure.in, following CF_WITH_MAN2HTML
define([CF_MAKE_DOCS],[
test -z "$cf_make_docs" && cf_make_docs=0

cf_output=makefile
test -f "$cf_output" || cf_output=Makefile

if test "$cf_make_docs" = 0
then
cat >>$cf_output <<CF_EOF
################################################################################
## generated by $0
.SUFFIXES : .html .$2 .man .ps .pdf .txt

${NROFF_NOTE}.$2.txt :
${NROFF_NOTE}	[\$](SHELL) -c "tbl [\$]*.$2 | nroff -man | col -bx" >[\$]@

${GROFF_NOTE}.ps.pdf :
${GROFF_NOTE}	ps2pdf [\$]*.ps
${GROFF_NOTE}
${GROFF_NOTE}.$2.ps :
${GROFF_NOTE}	[\$](SHELL) -c "tbl [\$]*.$2 | groff -man" >[\$]@
${GROFF_NOTE}
${GROFF_NOTE}.$2.txt :
${GROFF_NOTE}	GROFF_NO_SGR=stupid [\$](SHELL) -c "tbl [\$]*.$2 | nroff -rHY=0 -Tascii -man | col -bx" >[\$]@

${MAN2HTML_NOTE}.$2.html :
${MAN2HTML_NOTE}	./${MAN2HTML_TEMP} [\$]* $2 man >[\$]@

CF_EOF
	cf_make_docs=1
fi

for cf_name in $1
do
cat >>$cf_output <<CF_EOF
################################################################################
${NROFF_NOTE}docs docs-$cf_name :: $cf_name.txt
${MAN2HTML_NOTE}docs docs-$cf_name :: $cf_name.html
${GROFF_NOTE}docs docs-$cf_name :: $cf_name.pdf
${GROFF_NOTE}docs docs-$cf_name :: $cf_name.ps
${GROFF_NOTE}docs docs-$cf_name :: $cf_name.txt

clean \\
docs-clean ::
${NROFF_NOTE}	rm -f $cf_name.txt
${MAN2HTML_NOTE}	rm -f $cf_name.html
${GROFF_NOTE}	rm -f $cf_name.pdf
${GROFF_NOTE}	rm -f $cf_name.ps
${GROFF_NOTE}	rm -f $cf_name.txt

${NROFF_NOTE}$cf_name.txt  : $cf_name.$2
${MAN2HTML_NOTE}$cf_name.html : $cf_name.$2
${GROFF_NOTE}$cf_name.pdf  : $cf_name.ps
${GROFF_NOTE}$cf_name.ps   : $cf_name.$2
${GROFF_NOTE}$cf_name.txt  : $cf_name.$2
CF_EOF
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKE_TAGS version: 6 updated: 2010/10/23 15:52:32
dnl ------------
dnl Generate tags/TAGS targets for makefiles.  Do not generate TAGS if we have
dnl a monocase filesystem.
AC_DEFUN([CF_MAKE_TAGS],[
AC_REQUIRE([CF_MIXEDCASE_FILENAMES])

AC_CHECK_PROGS(CTAGS, exctags ctags)
AC_CHECK_PROGS(ETAGS, exetags etags)

AC_CHECK_PROG(MAKE_LOWER_TAGS, ${CTAGS:-ctags}, yes, no)

if test "$cf_cv_mixedcase" = yes ; then
	AC_CHECK_PROG(MAKE_UPPER_TAGS, ${ETAGS:-etags}, yes, no)
else
	MAKE_UPPER_TAGS=no
fi

if test "$MAKE_UPPER_TAGS" = yes ; then
	MAKE_UPPER_TAGS=
else
	MAKE_UPPER_TAGS="#"
fi

if test "$MAKE_LOWER_TAGS" = yes ; then
	MAKE_LOWER_TAGS=
else
	MAKE_LOWER_TAGS="#"
fi

AC_SUBST(CTAGS)
AC_SUBST(ETAGS)

AC_SUBST(MAKE_UPPER_TAGS)
AC_SUBST(MAKE_LOWER_TAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MIXEDCASE_FILENAMES version: 9 updated: 2021/01/01 16:53:59
dnl ----------------------
dnl Check if the file-system supports mixed-case filenames.  If we're able to
dnl create a lowercase name and see it as uppercase, it doesn't support that.
AC_DEFUN([CF_MIXEDCASE_FILENAMES],
[
AC_CACHE_CHECK(if filesystem supports mixed-case filenames,cf_cv_mixedcase,[
if test "$cross_compiling" = yes ; then
	case "$target_alias" in
	(*-os2-emx*|*-msdosdjgpp*|*-cygwin*|*-msys*|*-mingw*|*-uwin*|darwin*)
		cf_cv_mixedcase=no
		;;
	(*)
		cf_cv_mixedcase=yes
		;;
	esac
else
	rm -f conftest CONFTEST
	echo test >conftest
	if test -f CONFTEST ; then
		cf_cv_mixedcase=no
	else
		cf_cv_mixedcase=yes
	fi
	rm -f conftest CONFTEST
fi
])
test "$cf_cv_mixedcase" = yes && AC_DEFINE(MIXEDCASE_FILENAMES,1,[Define to 1 if filesystem supports mixed-case filenames.])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 5 updated: 2010/10/23 15:52:32
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "${as_me:-configure}:__oline__: testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NO_LEAKS_OPTION version: 9 updated: 2021/06/13 19:45:41
dnl ------------------
dnl see CF_WITH_NO_LEAKS
dnl
dnl $1 = option/name
dnl $2 = help-text
dnl $3 = symbol to define if the option is set
dnl $4 = additional actions to take if the option is set
AC_DEFUN([CF_NO_LEAKS_OPTION],[
AC_MSG_CHECKING(if you want to use $1 for testing)
AC_ARG_WITH($1,
	[$2],
	[case "x$withval" in
	(x|xno) ;;
	(*)
		: "${with_cflags:=-g}"
		: "${enable_leaks:=no}"
		with_$1=yes
		AC_DEFINE_UNQUOTED($3,1,"Define to 1 if you want to use $1 for testing.")ifelse([$4],,[
	 $4
])
		;;
	esac],
	[with_$1=])
AC_MSG_RESULT(${with_$1:-no})

case ".$with_cflags" in
(.*-g*)
	case .$CFLAGS in
	(.*-g*)
		;;
	(*)
		CF_ADD_CFLAGS([-g])
		;;
	esac
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATHSEP version: 8 updated: 2021/01/01 13:31:04
dnl ----------
dnl Provide a value for the $PATH and similar separator (or amend the value
dnl as provided in autoconf 2.5x).
AC_DEFUN([CF_PATHSEP],
[
	AC_MSG_CHECKING(for PATH separator)
	case "$cf_cv_system_name" in
	(os2*)	PATH_SEPARATOR=';'  ;;
	(*)	${PATH_SEPARATOR:=':'}  ;;
	esac
ifelse([$1],,,[$1=$PATH_SEPARATOR])
	AC_SUBST(PATH_SEPARATOR)
	AC_MSG_RESULT($PATH_SEPARATOR)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_SYNTAX version: 19 updated: 2024/08/03 13:08:58
dnl --------------
dnl Check the argument to see that it looks like a pathname.  Rewrite it if it
dnl begins with one of the prefix/exec_prefix variables, and then again if the
dnl result begins with 'NONE'.  This is necessary to work around autoconf's
dnl delayed evaluation of those symbols.
AC_DEFUN([CF_PATH_SYNTAX],[
AC_REQUIRE([CF_GLOB_FULLPATH])dnl

if test "x$prefix" != xNONE; then
	cf_path_syntax="$prefix"
else
	cf_path_syntax="$ac_default_prefix"
fi

case "x[$]$1" in
(x\[$]\(*\)*|x\'*\'*)
	;;
(x.|x$GLOB_FULLPATH_POSIX|x$GLOB_FULLPATH_OTHER)
	;;
(x\[$]\{*prefix\}*|x\[$]\{*dir\}*)
	eval $1="[$]$1"
	case "x[$]$1" in
	(xNONE/*)
		$1=`echo "[$]$1" | sed -e s%NONE%$cf_path_syntax%`
		;;
	esac
	;;
(xno|xNONE/*)
	$1=`echo "[$]$1" | sed -e s%NONE%$cf_path_syntax%`
	;;
(*)
	ifelse([$2],,[AC_MSG_ERROR([expected a pathname, not \"[$]$1\"])],$2)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PKG_CONFIG version: 13 updated: 2023/10/28 11:59:01
dnl -------------
dnl Check for the package-config program, unless disabled by command-line.
dnl
dnl Sets $PKG_CONFIG to the pathname of the pkg-config program.
AC_DEFUN([CF_PKG_CONFIG],
[
AC_MSG_CHECKING(if you want to use pkg-config)
AC_ARG_WITH(pkg-config,
	[[  --with-pkg-config[=CMD] enable/disable use of pkg-config and its name CMD]],
	[cf_pkg_config=$withval],
	[cf_pkg_config=yes])
AC_MSG_RESULT($cf_pkg_config)

case "$cf_pkg_config" in
(no)
	PKG_CONFIG=none
	;;
(yes)
	CF_ACVERSION_CHECK(2.52,
		[AC_PATH_TOOL(PKG_CONFIG, pkg-config, none)],
		[AC_PATH_PROG(PKG_CONFIG, pkg-config, none)])
	;;
(*)
	PKG_CONFIG=$withval
	;;
esac

test -z "$PKG_CONFIG" && PKG_CONFIG=none
if test "$PKG_CONFIG" != none ; then
	CF_PATH_SYNTAX(PKG_CONFIG)
elif test "x$cf_pkg_config" != xno ; then
	AC_MSG_WARN(pkg-config is not installed)
fi

AC_SUBST(PKG_CONFIG)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_C_SOURCE version: 12 updated: 2023/02/18 17:41:25
dnl -----------------
dnl Define _POSIX_C_SOURCE to the given level, and _POSIX_SOURCE if needed.
dnl
dnl	POSIX.1-1990				_POSIX_SOURCE
dnl	POSIX.1-1990 and			_POSIX_SOURCE and
dnl		POSIX.2-1992 C-Language			_POSIX_C_SOURCE=2
dnl		Bindings Option
dnl	POSIX.1b-1993				_POSIX_C_SOURCE=199309L
dnl	POSIX.1c-1996				_POSIX_C_SOURCE=199506L
dnl	X/Open 2000				_POSIX_C_SOURCE=200112L
dnl
dnl Parameters:
dnl	$1 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_POSIX_C_SOURCE],
[AC_REQUIRE([CF_POSIX_VISIBLE])dnl

if test "$cf_cv_posix_visible" = no; then

cf_POSIX_C_SOURCE=ifelse([$1],,199506L,[$1])

cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"

CF_REMOVE_DEFINE(cf_trim_CFLAGS,$cf_save_CFLAGS,_POSIX_C_SOURCE)
CF_REMOVE_DEFINE(cf_trim_CPPFLAGS,$cf_save_CPPFLAGS,_POSIX_C_SOURCE)

AC_CACHE_CHECK(if we should define _POSIX_C_SOURCE,cf_cv_posix_c_source,[
	CF_MSG_LOG(if the symbol is already defined go no further)
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
#error _POSIX_C_SOURCE is not defined
#endif],
	[cf_cv_posix_c_source=no],
	[cf_want_posix_source=no
	 case .$cf_POSIX_C_SOURCE in
	 (.[[12]]??*)
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		;;
	 (.2)
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		cf_want_posix_source=yes
		;;
	 (.*)
		cf_want_posix_source=yes
		;;
	 esac
	 if test "$cf_want_posix_source" = yes ; then
		AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _POSIX_SOURCE
#error _POSIX_SOURCE is defined
#endif],[],
		cf_cv_posix_c_source="$cf_cv_posix_c_source -D_POSIX_SOURCE")
	 fi
	 CF_MSG_LOG(ifdef from value $cf_POSIX_C_SOURCE)
	 CFLAGS="$cf_trim_CFLAGS"
	 CPPFLAGS="$cf_trim_CPPFLAGS"
	 CF_APPEND_TEXT(CPPFLAGS,$cf_cv_posix_c_source)
	 CF_MSG_LOG(if the second compile does not leave our definition intact error)
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
#error _POSIX_C_SOURCE is not defined
#endif],,
	 [cf_cv_posix_c_source=no])
	 CFLAGS="$cf_save_CFLAGS"
	 CPPFLAGS="$cf_save_CPPFLAGS"
	])
])

if test "$cf_cv_posix_c_source" != no ; then
	CFLAGS="$cf_trim_CFLAGS"
	CPPFLAGS="$cf_trim_CPPFLAGS"
	CF_ADD_CFLAGS($cf_cv_posix_c_source)
fi

fi # cf_cv_posix_visible

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_VISIBLE version: 1 updated: 2018/12/31 20:46:17
dnl ----------------
dnl POSIX documents test-macros which an application may set before any system
dnl headers are included to make features available.
dnl
dnl Some BSD platforms (originally FreeBSD, but copied by a few others)
dnl diverged from POSIX in 2002 by setting symbols which make all of the most
dnl recent features visible in the system header files unless the application
dnl overrides the corresponding test-macros.  Doing that introduces portability
dnl problems.
dnl
dnl This macro makes a special check for the symbols used for this, to avoid a
dnl conflicting definition.
AC_DEFUN([CF_POSIX_VISIBLE],
[
AC_CACHE_CHECK(if the POSIX test-macros are already defined,cf_cv_posix_visible,[
AC_TRY_COMPILE([#include <stdio.h>],[
#if defined(__POSIX_VISIBLE) && ((__POSIX_VISIBLE - 0L) > 0) \
	&& defined(__XSI_VISIBLE) && ((__XSI_VISIBLE - 0L) > 0) \
	&& defined(__BSD_VISIBLE) && ((__BSD_VISIBLE - 0L) > 0) \
	&& defined(__ISO_C_VISIBLE) && ((__ISO_C_VISIBLE - 0L) > 0)
#error conflicting symbols found
#endif
],[cf_cv_posix_visible=no],[cf_cv_posix_visible=yes])
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_CC version: 5 updated: 2019/12/31 08:53:54
dnl ----------
dnl standard check for CC, plus followup sanity checks
dnl $1 = optional parameter to pass to AC_PROG_CC to specify compiler name
AC_DEFUN([CF_PROG_CC],[
CF_ACVERSION_CHECK(2.53,
	[AC_MSG_WARN(this will incorrectly handle gnatgcc choice)
	 AC_REQUIRE([AC_PROG_CC])],
	[])
ifelse($1,,[AC_PROG_CC],[AC_PROG_CC($1)])
CF_GCC_VERSION
CF_ACVERSION_CHECK(2.52,
	[AC_PROG_CC_STDC],
	[CF_ANSI_CC_REQD])
CF_CC_ENV_FLAGS
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_GROFF version: 3 updated: 2018/01/07 13:16:19
dnl -------------
dnl Check if groff is available, for cases (such as html output) where nroff
dnl is not enough.
AC_DEFUN([CF_PROG_GROFF],[
AC_PATH_PROG(GROFF_PATH,groff,no)
AC_PATH_PROGS(NROFF_PATH,nroff mandoc,no)
AC_PATH_PROG(TBL_PATH,tbl,cat)
if test "x$GROFF_PATH" = xno
then
	NROFF_NOTE=
	GROFF_NOTE="#"
else
	NROFF_NOTE="#"
	GROFF_NOTE=
fi
AC_SUBST(GROFF_NOTE)
AC_SUBST(NROFF_NOTE)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_LINT version: 7 updated: 2024/11/30 14:37:45
dnl ------------
AC_DEFUN([CF_PROG_LINT],
[
AC_CHECK_PROGS(LINT, lint cppcheck splint)
case "x$LINT" in
(xlint|x*/lint) # NetBSD 10
	test -z "$LINT_OPTS" && LINT_OPTS="-chapbrxzgFS -v -Ac11"
	;;
(xcppcheck|x*/cppcheck)
	test -z "$LINT_OPTS" && LINT_OPTS="--enable=all -D__CPPCHECK__"
	;;
esac
AC_SUBST(LINT_OPTS)
AC_SUBST(LINT_LIBS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_CFLAGS version: 3 updated: 2021/09/05 17:25:40
dnl ----------------
dnl Remove a given option from CFLAGS/CPPFLAGS
dnl $1 = option to remove
dnl $2 = variable to update
dnl $3 = nonempty to allow verbose message
define([CF_REMOVE_CFLAGS],
[
cf_tmp_cflag=`echo "x$1" | sed -e 's/^.//' -e 's/=.*//'`
while true
do
	cf_old_cflag=`echo "x[$]$2" | sed -e 's/^.//' -e 's/[[ 	]][[ 	]]*-/ -/g' -e "s%$cf_tmp_cflag\\(=[[^ 	]][[^ 	]]*\\)\?%%" -e 's/^[[ 	]]*//' -e 's%[[ ]][[ ]]*-D% -D%g' -e 's%[[ ]][[ ]]*-I% -I%g'`
	test "[$]$2" != "$cf_old_cflag" || break
	ifelse([$3],,,[CF_VERBOSE(removing old option $1 from $2)])
	$2="$cf_old_cflag"
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_DEFINE version: 3 updated: 2010/01/09 11:05:50
dnl ----------------
dnl Remove all -U and -D options that refer to the given symbol from a list
dnl of C compiler options.  This works around the problem that not all
dnl compilers process -U and -D options from left-to-right, so a -U option
dnl cannot be used to cancel the effect of a preceding -D option.
dnl
dnl $1 = target (which could be the same as the source variable)
dnl $2 = source (including '$')
dnl $3 = symbol to remove
define([CF_REMOVE_DEFINE],
[
$1=`echo "$2" | \
	sed	-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[[ 	]]/ /g' \
		-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[$]//g'`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RESTORE_XTRA_FLAGS version: 1 updated: 2020/01/11 16:47:45
dnl ---------------------
dnl Restore flags saved in CF_SAVE_XTRA_FLAGS
dnl $1 = name of current macro
define([CF_RESTORE_XTRA_FLAGS],
[
LIBS="$cf_save_LIBS_$1"
CFLAGS="$cf_save_CFLAGS_$1"
CPPFLAGS="$cf_save_CPPFLAGS_$1"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RPATH_HACK version: 13 updated: 2021/01/03 18:30:50
dnl -------------
AC_DEFUN([CF_RPATH_HACK],
[AC_REQUIRE([AC_PROG_FGREP])dnl
AC_REQUIRE([CF_LD_RPATH_OPT])dnl

AC_MSG_CHECKING(for updated LDFLAGS)
if test -n "$LD_RPATH_OPT" ; then
	AC_MSG_RESULT(maybe)

	AC_CHECK_PROGS(cf_ldd_prog,ldd,no)
	cf_rpath_list="/usr/lib /lib"
	if test "$cf_ldd_prog" != no
	then
		cf_rpath_oops=

AC_TRY_LINK([#include <stdio.h>],
		[printf("Hello");],
		[cf_rpath_oops=`"$cf_ldd_prog" "conftest$ac_exeext" | ${FGREP-fgrep} ' not found' | sed -e 's% =>.*$%%' |sort | uniq`
		 cf_rpath_list=`"$cf_ldd_prog" "conftest$ac_exeext" | ${FGREP-fgrep} / | sed -e 's%^.*[[ 	]]/%/%' -e 's%/[[^/]][[^/]]*$%%' |sort | uniq`])

		# If we passed the link-test, but get a "not found" on a given library,
		# this could be due to inept reconfiguration of gcc to make it only
		# partly honor /usr/local/lib (or whatever).  Sometimes this behavior
		# is intentional, e.g., installing gcc in /usr/bin and suppressing the
		# /usr/local libraries.
		if test -n "$cf_rpath_oops"
		then
			for cf_rpath_src in $cf_rpath_oops
			do
				for cf_rpath_dir in \
					/usr/local \
					/usr/pkg \
					/opt/sfw
				do
					if test -f "$cf_rpath_dir/lib/$cf_rpath_src"
					then
						CF_VERBOSE(...adding -L$cf_rpath_dir/lib to LDFLAGS for $cf_rpath_src)
						LDFLAGS="$LDFLAGS -L$cf_rpath_dir/lib"
						break
					fi
				done
			done
		fi
	fi

	CF_VERBOSE(...checking EXTRA_LDFLAGS $EXTRA_LDFLAGS)

	CF_RPATH_HACK_2(LDFLAGS)
	CF_RPATH_HACK_2(LIBS)

	CF_VERBOSE(...checked EXTRA_LDFLAGS $EXTRA_LDFLAGS)
else
	AC_MSG_RESULT(no)
fi
AC_SUBST(EXTRA_LDFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RPATH_HACK_2 version: 8 updated: 2021/01/01 13:31:04
dnl ---------------
dnl Do one set of substitutions for CF_RPATH_HACK, adding an rpath option to
dnl EXTRA_LDFLAGS for each -L option found.
dnl
dnl $cf_rpath_list contains a list of directories to ignore.
dnl
dnl $1 = variable name to update.  The LDFLAGS variable should be the only one,
dnl      but LIBS often has misplaced -L options.
AC_DEFUN([CF_RPATH_HACK_2],
[
CF_VERBOSE(...checking $1 [$]$1)

cf_rpath_dst=
for cf_rpath_src in [$]$1
do
	case "$cf_rpath_src" in
	(-L*)

		# check if this refers to a directory which we will ignore
		cf_rpath_skip=no
		if test -n "$cf_rpath_list"
		then
			for cf_rpath_item in $cf_rpath_list
			do
				if test "x$cf_rpath_src" = "x-L$cf_rpath_item"
				then
					cf_rpath_skip=yes
					break
				fi
			done
		fi

		if test "$cf_rpath_skip" = no
		then
			# transform the option
			if test "$LD_RPATH_OPT" = "-R " ; then
				cf_rpath_tmp=`echo "$cf_rpath_src" |sed -e "s%-L%-R %"`
			else
				cf_rpath_tmp=`echo "$cf_rpath_src" |sed -e "s%-L%$LD_RPATH_OPT%"`
			fi

			# if we have not already added this, add it now
			cf_rpath_tst=`echo "$EXTRA_LDFLAGS" | sed -e "s%$cf_rpath_tmp %%"`
			if test "x$cf_rpath_tst" = "x$EXTRA_LDFLAGS"
			then
				CF_VERBOSE(...Filter $cf_rpath_src ->$cf_rpath_tmp)
				EXTRA_LDFLAGS="$cf_rpath_tmp $EXTRA_LDFLAGS"
			fi
		fi
		;;
	esac
	cf_rpath_dst="$cf_rpath_dst $cf_rpath_src"
done
$1=$cf_rpath_dst

CF_VERBOSE(...checked $1 [$]$1)
AC_SUBST(EXTRA_LDFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SAVE_XTRA_FLAGS version: 1 updated: 2020/01/11 16:46:44
dnl ------------------
dnl Use this macro to save CFLAGS/CPPFLAGS/LIBS before checks against X headers
dnl and libraries which do not update those variables.
dnl
dnl $1 = name of current macro
define([CF_SAVE_XTRA_FLAGS],
[
cf_save_LIBS_$1="$LIBS"
cf_save_CFLAGS_$1="$CFLAGS"
cf_save_CPPFLAGS_$1="$CPPFLAGS"
LIBS="$LIBS ${X_PRE_LIBS} ${X_LIBS} ${X_EXTRA_LIBS}"
for cf_X_CFLAGS in $X_CFLAGS
do
	case "x$cf_X_CFLAGS" in
	x-[[IUD]]*)
		CPPFLAGS="$CPPFLAGS $cf_X_CFLAGS"
		;;
	*)
		CFLAGS="$CFLAGS $cf_X_CFLAGS"
		;;
	esac
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIGWINCH version: 7 updated: 2023/02/18 17:41:25
dnl -----------
dnl Use this macro after CF_XOPEN_SOURCE, but do not require it (not all
dnl programs need this test).
dnl
dnl This is really a Mac OS X 10.4.3 workaround.  Defining _POSIX_C_SOURCE
dnl forces SIGWINCH to be undefined (breaks xterm, ncurses).  Oddly, the struct
dnl winsize declaration is left alone - we may revisit this if Apple choose to
dnl break that part of the interface as well.
AC_DEFUN([CF_SIGWINCH],
[
AC_CACHE_CHECK(if SIGWINCH is defined,cf_cv_define_sigwinch,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH; (void)x],
	[cf_cv_define_sigwinch=yes],
	[AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH; (void)x],
	[cf_cv_define_sigwinch=maybe],
	[cf_cv_define_sigwinch=no])
])
])

if test "$cf_cv_define_sigwinch" = maybe ; then
AC_CACHE_CHECK(for actual SIGWINCH definition,cf_cv_fixup_sigwinch,[
cf_cv_fixup_sigwinch=unknown
cf_sigwinch=32
while test "$cf_sigwinch" != 1
do
	AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[
#if SIGWINCH != $cf_sigwinch
#error SIGWINCH is not $cf_sigwinch
#endif
int x = SIGWINCH; (void)x],
	[cf_cv_fixup_sigwinch=$cf_sigwinch
	 break])

cf_sigwinch="`expr "$cf_sigwinch" - 1`"
done
])

	if test "$cf_cv_fixup_sigwinch" != unknown ; then
		CPPFLAGS="$CPPFLAGS -DSIGWINCH=$cf_cv_fixup_sigwinch"
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SUBDIR_PATH version: 7 updated: 2014/12/04 04:33:06
dnl --------------
dnl Construct a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
AC_DEFUN([CF_SUBDIR_PATH],
[
$1=

CF_ADD_SUBDIR_PATH($1,$2,$3,$prefix,NONE)

for cf_subdir_prefix in \
	/usr \
	/usr/local \
	/usr/pkg \
	/opt \
	/opt/local \
	[$]HOME
do
	CF_ADD_SUBDIR_PATH($1,$2,$3,$cf_subdir_prefix,$prefix)
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SVR4 version: 6 updated: 2023/12/01 17:22:50
dnl -------
dnl Check if this is an SVR4 system.  We need the definition for xterm
AC_DEFUN([CF_SVR4],
[
AC_CHECK_LIB(elf, elf_begin,[
AC_CACHE_CHECK(if this is an SVR4 system, cf_cv_svr4,[
AC_TRY_COMPILE([
#if defined(__CYGWIN__)
#error Cygwin is not SVr4
#endif
#include <elf.h>
#include <sys/termio.h>
],[
static struct termio d_tio;
	d_tio.c_cc[VINTR] = 0;
	d_tio.c_cc[VQUIT] = 0;
	d_tio.c_cc[VERASE] = 0;
	d_tio.c_cc[VKILL] = 0;
	d_tio.c_cc[VEOF] = 0;
	d_tio.c_cc[VEOL] = 0;
	d_tio.c_cc[VMIN] = 0;
	d_tio.c_cc[VTIME] = 0;
	d_tio.c_cc[VLNEXT] = 0;
],
[cf_cv_svr4=yes],
[cf_cv_svr4=no])
])
])
test "$cf_cv_svr4" = yes && AC_DEFINE(SVR4,1,[Define to 1 if this is an SVR4 system])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYSV version: 16 updated: 2020/01/11 16:46:44
dnl -------
dnl Check if this is a SYSV platform, e.g., as used in <X11/Xos.h>, and whether
dnl defining it will be helpful.  The following features are used to check:
dnl
dnl a) bona-fide SVSV doesn't use const for sys_errlist[].  Since this is a
dnl legacy (pre-ANSI) feature, const should not apply.  Modern systems only
dnl declare strerror().  Xos.h declares the legacy form of str_errlist[], and
dnl a compile-time error will result from trying to assign to a const array.
dnl
dnl b) compile with headers that exist on SYSV hosts.
dnl
dnl c) compile with type definitions that differ on SYSV hosts from standard C.
AC_DEFUN([CF_SYSV],
[
AC_REQUIRE([AC_PATH_XTRA])

CF_SAVE_XTRA_FLAGS([CF_SYSV])

AC_CHECK_HEADERS( \
termios.h \
stdlib.h \
X11/Intrinsic.h \
)

AC_REQUIRE([CF_SYS_ERRLIST])

AC_CACHE_CHECK(if we should define SYSV,cf_cv_sysv,[
AC_TRY_COMPILE([
#undef  SYSV
#define SYSV 1			/* get Xos.h to declare sys_errlist[] */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>		/* look for wchar_t */
#endif
#ifdef HAVE_X11_INTRINSIC_H
#include <X11/Intrinsic.h>	/* Intrinsic.h has other traps... */
#endif
#ifdef HAVE_TERMIOS_H		/* needed for HPUX 10.20 */
#include <termios.h>
#define STRUCT_TERMIOS struct termios
#else
#define STRUCT_TERMIOS struct termio
#endif
#include <curses.h>
#include <term.h>		/* eliminate most BSD hacks */
#include <errno.h>		/* declare sys_errlist on older systems */
#include <sys/termio.h>		/* eliminate most of the remaining ones */
],[
static STRUCT_TERMIOS d_tio;
	d_tio.c_cc[VINTR] = 0;
	d_tio.c_cc[VQUIT] = 0;
	d_tio.c_cc[VERASE] = 0;
	d_tio.c_cc[VKILL] = 0;
	d_tio.c_cc[VEOF] = 0;
	d_tio.c_cc[VEOL] = 0;
	d_tio.c_cc[VMIN] = 0;
	d_tio.c_cc[VTIME] = 0;
#if defined(HAVE_SYS_ERRLIST) && !defined(DECL_SYS_ERRLIST)
sys_errlist[0] = "";		/* Cygwin mis-declares this */
#endif
],
[cf_cv_sysv=yes],
[cf_cv_sysv=no])
])
test "$cf_cv_sysv" = yes && AC_DEFINE(SYSV,1,[Define to 1 if this is an SYSV system])

CF_RESTORE_XTRA_FLAGS([CF_SYSV])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYS_ERRLIST version: 6 updated: 2001/12/30 13:03:23
dnl --------------
dnl Check for declaration of sys_nerr and sys_errlist in one of stdio.h and
dnl errno.h.  Declaration of sys_errlist on BSD4.4 interferes with our
dnl declaration.  Reported by Keith Bostic.
AC_DEFUN([CF_SYS_ERRLIST],
[
    CF_CHECK_ERRNO(sys_nerr)
    CF_CHECK_ERRNO(sys_errlist)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYS_TIME_SELECT version: 6 updated: 2015/04/18 08:56:57
dnl ------------------
dnl Check if we can include <sys/time.h> with <sys/select.h>; this breaks on
dnl older SCO configurations.
AC_DEFUN([CF_SYS_TIME_SELECT],
[
AC_MSG_CHECKING(if sys/time.h works with sys/select.h)
AC_CACHE_VAL(cf_cv_sys_time_select,[
AC_TRY_COMPILE([
#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
],[],[cf_cv_sys_time_select=yes],
	 [cf_cv_sys_time_select=no])
	 ])
AC_MSG_RESULT($cf_cv_sys_time_select)
test "$cf_cv_sys_time_select" = yes && AC_DEFINE(HAVE_SYS_TIME_SELECT,1,[Define to 1 if we can include <sys/time.h> with <sys/select.h>])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TRY_XOPEN_SOURCE version: 4 updated: 2022/09/10 15:16:16
dnl -------------------
dnl If _XOPEN_SOURCE is not defined in the compile environment, check if we
dnl can define it successfully.
AC_DEFUN([CF_TRY_XOPEN_SOURCE],[
AC_CACHE_CHECK(if we should define _XOPEN_SOURCE,cf_cv_xopen_source,[
	AC_TRY_COMPILE(CF__XOPEN_SOURCE_HEAD,CF__XOPEN_SOURCE_BODY,
	[cf_cv_xopen_source=no],
	[cf_save="$CPPFLAGS"
	 CF_APPEND_TEXT(CPPFLAGS,-D_XOPEN_SOURCE=$cf_XOPEN_SOURCE)
	 AC_TRY_COMPILE(CF__XOPEN_SOURCE_HEAD,CF__XOPEN_SOURCE_BODY,
		[cf_cv_xopen_source=no],
		[cf_cv_xopen_source=$cf_XOPEN_SOURCE])
		CPPFLAGS="$cf_save"
	])
])

if test "$cf_cv_xopen_source" != no ; then
	CF_REMOVE_DEFINE(CFLAGS,$CFLAGS,_XOPEN_SOURCE)
	CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,_XOPEN_SOURCE)
	cf_temp_xopen_source="-D_XOPEN_SOURCE=$cf_cv_xopen_source"
	CF_APPEND_CFLAGS($cf_temp_xopen_source)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_UPPER version: 5 updated: 2001/01/29 23:40:59
dnl --------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 3 updated: 2007/07/29 09:55:12
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
CF_MSG_LOG([$1])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITHOUT_X version: 3 updated: 2021/01/13 16:51:52
dnl ------------
dnl Use this to cancel the check for X headers/libraries which would be pulled
dnl in via CF_GCC_WARNINGS.
define([CF_WITHOUT_X],
AC_DEFUN([AC_PATH_XTRA],[])
AC_DEFUN([CF_SAVE_XTRA_FLAGS],[])
AC_DEFUN([CF_RESTORE_XTRA_FLAGS],[])
AC_DEFUN([CF_CONST_X_STRING],[echo "skipping X-const check";])dnl
AC_SUBST(X_CFLAGS)
AC_SUBST(X_LIBS)
[])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DBMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ----------------
dnl Configure-option for dbmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DBMALLOC],[
CF_NO_LEAKS_OPTION(dbmalloc,
	[  --with-dbmalloc         test: use Conor Cahill's dbmalloc library],
	[USE_DBMALLOC])

if test "$with_dbmalloc" = yes ; then
	AC_CHECK_HEADER(dbmalloc.h,
		[AC_CHECK_LIB(dbmalloc,[debug_malloc]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ---------------
dnl Configure-option for dmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DMALLOC],[
CF_NO_LEAKS_OPTION(dmalloc,
	[  --with-dmalloc          test: use Gray Watson's dmalloc library],
	[USE_DMALLOC])

if test "$with_dmalloc" = yes ; then
	AC_CHECK_HEADER(dmalloc.h,
		[AC_CHECK_LIB(dmalloc,[dmalloc_debug]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_ENCODINGS_DIR version: 7 updated: 2023/11/22 19:48:06
dnl ---------------------
dnl Configure option to specify the location of encodings.dir, for programs
dnl that must read it directly.
AC_DEFUN([CF_WITH_ENCODINGS_DIR],
[
AC_MSG_CHECKING(for location of encodings.dir file)
AC_ARG_WITH(encodings-dir,
[[  --with-encodings-dir[=XXX] file used for encodings directory (default: auto)]],
[ENCODINGS_DIR_FILE=$withval],
[ENCODINGS_DIR_FILE=auto])

CF_VERBOSE()
case "x$ENCODINGS_DIR_FILE" in
(xauto|xyes|xno)
	ENCODINGS_DIR_FILE=unknown
	for cf_leaf in lib share
	do
		for cf_path in \
			/usr/X11R7/$cf_leaf/X11/fonts \
			/usr/X11R6/$cf_leaf/X11/fonts \
			/usr/local/$cf_leaf/X11/fonts \
			/opt/local/$cf_leaf/fonts/X11 \
			/opt/X11/$cf_leaf/X11/fonts \
			/usr/$cf_leaf/fonts/X11 \
			/usr/$cf_leaf/X11/fonts
		do
			test -d "$cf_path" || continue
			cf_encodings_dir=$cf_path/encodings/encodings.dir
			CF_VERBOSE(...testing $cf_encodings_dir)
			if test -f $cf_encodings_dir ; then
				ENCODINGS_DIR_FILE=$cf_encodings_dir
				break
			fi
		done
	done
	;;
(*)
	if test ! -f "$ENCODINGS_DIR_FILE" ; then
		AC_MSG_WARN(Encodings file not found: $ENCODINGS_DIR_FILE)
		ENCODINGS_DIR_FILE=/usr/share/fonts/X11
	fi
	;;
esac
AC_MSG_RESULT($ENCODINGS_DIR_FILE)
test $ENCODINGS_DIR_FILE = unknown && ENCODINGS_DIR_FILE=
AC_SUBST(ENCODINGS_DIR_FILE)
AC_DEFINE_UNQUOTED(ENCODINGS_DIR_FILE, "$ENCODINGS_DIR_FILE", [Define to pathname of file used for encodings directory])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_LOCALE_ALIAS version: 10 updated: 2023/11/22 19:50:11
dnl --------------------
dnl Configure option to specify the location of locale.alias, for programs that
dnl must read it directly.
AC_DEFUN([CF_WITH_LOCALE_ALIAS],
[
AC_MSG_CHECKING(for location of locale alias file)
AC_ARG_WITH(locale-alias,
[[  --with-locale-alias[=XXX] file used for locale aliases (default: auto)]],
[LOCALE_ALIAS_FILE=$withval],
[LOCALE_ALIAS_FILE=auto])

CF_VERBOSE()
case "x$LOCALE_ALIAS_FILE" in
(xauto|xyes|xno)
	LOCALE_ALIAS_FILE=unknown
	for cf_leaf in lib share
	do
		for cf_path in \
			/usr/openwin/$cf_leaf/locale \
			/usr/$cf_leaf/X11/locale \
			/usr/X11R7/$cf_leaf/X11/locale \
			/usr/X11R6/$cf_leaf/X11/locale \
			/opt/local/$cf_leaf/X11/locale \
			/opt/X11/$cf_leaf/X11/locale \
			/usr/X11/$cf_leaf/X11/locale
		do
			test -d $cf_path || continue
			cf_alias_file=$cf_path/locale.alias
			CF_VERBOSE(testing $cf_alias_file)
			if test -f $cf_alias_file ; then
				LOCALE_ALIAS_FILE=$cf_alias_file
				break
			fi
		done
		test "$LOCALE_ALIAS_FILE" = unknown || break
	done
	;;
(*)
	if test ! -f "$LOCALE_ALIAS_FILE" ; then
		AC_MSG_WARN(Alias file not found: $LOCALE_ALIAS_FILE)
	fi
	;;
esac
AC_MSG_RESULT($LOCALE_ALIAS_FILE)
AC_SUBST(LOCALE_ALIAS_FILE)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_MAN2HTML version: 14 updated: 2024/09/09 17:17:46
dnl ----------------
dnl Check for man2html and groff.  Prefer man2html over groff, but use groff
dnl as a fallback.  See
dnl
dnl		https://invisible-island.net/scripts/man2html.html
dnl
dnl Generate a shell script which hides the differences between the two.
dnl
dnl We name that "man2html.tmp".
dnl
dnl The shell script can be removed later, e.g., using "make distclean".
AC_DEFUN([CF_WITH_MAN2HTML],[
AC_REQUIRE([CF_PROG_GROFF])dnl
AC_REQUIRE([AC_PROG_FGREP])dnl

case "x${with_man2html}" in
(xno)
	cf_man2html=no
	;;
(x|xyes)
	AC_PATH_PROG(cf_man2html,man2html,no)
	case "x$cf_man2html" in
	(x/*)
		AC_MSG_CHECKING(for the modified Earl Hood script)
		if ( $cf_man2html -help 2>&1 | grep 'Make an index of headers at the end' >/dev/null )
		then
			cf_man2html_ok=yes
		else
			cf_man2html=no
			cf_man2html_ok=no
		fi
		AC_MSG_RESULT($cf_man2html_ok)
		;;
	(*)
		cf_man2html=no
		;;
	esac
esac

AC_MSG_CHECKING(for program to convert manpage to html)
AC_ARG_WITH(man2html,
	[[  --with-man2html[=XXX]   use XXX rather than groff]],
	[cf_man2html=$withval],
	[cf_man2html=$cf_man2html])

cf_with_groff=no

case $cf_man2html in
(yes)
	AC_MSG_RESULT(man2html)
	AC_PATH_PROG(cf_man2html,man2html,no)
	;;
(no|groff|*/groff*)
	cf_with_groff=yes
	cf_man2html=$GROFF_PATH
	AC_MSG_RESULT($cf_man2html)
	;;
(*)
	AC_MSG_RESULT($cf_man2html)
	;;
esac

MAN2HTML_TEMP="man2html.tmp"
	cat >$MAN2HTML_TEMP <<CF_EOF
#!$SHELL
# Temporary script generated by CF_WITH_MAN2HTML
# Convert inputs to html, sending result to standard output.
#
# Parameters:
# \${1} = rootname of file to convert
# \${2} = suffix of file to convert, e.g., "1"
# \${3} = macros to use, e.g., "man"
#
ROOT=\[$]1
TYPE=\[$]2
MACS=\[$]3

unset LANG
unset LC_ALL
unset LC_CTYPE
unset LANGUAGE
GROFF_NO_SGR=stupid
export GROFF_NO_SGR

CF_EOF

NROFF_OPTS=
if test "x$cf_with_groff" = xyes
then
	MAN2HTML_NOTE="$GROFF_NOTE"
	MAN2HTML_PATH="$GROFF_PATH"
	cat >>$MAN2HTML_TEMP <<CF_EOF
$SHELL -c "$TBL_PATH \${ROOT}.\${TYPE} | $GROFF_PATH -P -o0 -I\${ROOT}_ -Thtml -\${MACS}"
CF_EOF
else
	# disable hyphenation if this is groff
	if test "x$GROFF_PATH" != xno
	then
		AC_MSG_CHECKING(if nroff is really groff)
		cf_check_groff="`$NROFF_PATH --version 2>/dev/null | grep groff`"
		test -n "$cf_check_groff" && cf_check_groff=yes
		test -n "$cf_check_groff" || cf_check_groff=no
		AC_MSG_RESULT($cf_check_groff)
		test "x$cf_check_groff" = xyes && NROFF_OPTS="-rHY=0"
	fi
	MAN2HTML_NOTE=""
	CF_PATH_SYNTAX(cf_man2html)
	MAN2HTML_PATH="$cf_man2html"
	AC_MSG_CHECKING(for $cf_man2html top/bottom margins)

	# for this example, expect 3 lines of content, the remainder is head/foot
	cat >conftest.in <<CF_EOF
.TH HEAD1 HEAD2 HEAD3 HEAD4 HEAD5
.SH SECTION
MARKER
CF_EOF

	LC_ALL=C LC_CTYPE=C LANG=C LANGUAGE=C $NROFF_PATH -man conftest.in >conftest.out

	cf_man2html_1st="`${FGREP-fgrep} -n MARKER conftest.out |sed -e 's/^[[^0-9]]*://' -e 's/:.*//'`"
	cf_man2html_top=`expr "$cf_man2html_1st" - 2`
	cf_man2html_bot="`wc -l conftest.out |sed -e 's/[[^0-9]]//g'`"
	cf_man2html_bot=`expr "$cf_man2html_bot" - 2 - "$cf_man2html_top"`
	cf_man2html_top_bot="-topm=$cf_man2html_top -botm=$cf_man2html_bot"

	AC_MSG_RESULT($cf_man2html_top_bot)

	AC_MSG_CHECKING(for pagesize to use)
	for cf_block in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
	do
	cat >>conftest.in <<CF_EOF
.nf
0
1
2
3
4
5
6
7
8
9
CF_EOF
	done

	LC_ALL=C LC_CTYPE=C LANG=C LANGUAGE=C $NROFF_PATH -man conftest.in >conftest.out
	cf_man2html_page="`${FGREP-fgrep} -n HEAD1 conftest.out |sed -n '$p' |sed -e 's/^[[^0-9]]*://' -e 's/:.*//'`"
	test -z "$cf_man2html_page" && cf_man2html_page=99999
	test "$cf_man2html_page" -gt 100 && cf_man2html_page=99999

	rm -rf conftest*
	AC_MSG_RESULT($cf_man2html_page)

	cat >>$MAN2HTML_TEMP <<CF_EOF
: \${MAN2HTML_PATH=$MAN2HTML_PATH}
MAN2HTML_OPTS="\$MAN2HTML_OPTS -index -title=\"\$ROOT(\$TYPE)\" -compress -pgsize $cf_man2html_page"
case \${TYPE} in
(ms)
	$TBL_PATH \${ROOT}.\${TYPE} | $NROFF_PATH $NROFF_OPTS -\${MACS} | \$MAN2HTML_PATH -topm=0 -botm=0 \$MAN2HTML_OPTS
	;;
(*)
	$TBL_PATH \${ROOT}.\${TYPE} | $NROFF_PATH $NROFF_OPTS -\${MACS} | \$MAN2HTML_PATH $cf_man2html_top_bot \$MAN2HTML_OPTS
	;;
esac
CF_EOF
fi

chmod 700 $MAN2HTML_TEMP

AC_SUBST(MAN2HTML_NOTE)
AC_SUBST(MAN2HTML_PATH)
AC_SUBST(MAN2HTML_TEMP)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_SYSTYPE version: 1 updated: 2013/01/26 16:26:12
dnl ---------------
dnl For testing, override the derived host system-type which is used to decide
dnl things such as the linker commands used to build shared libraries.  This is
dnl normally chosen automatically based on the type of system which you are
dnl building on.  We use it for testing the configure script.
dnl
dnl This is different from the --host option: it is used only for testing parts
dnl of the configure script which would not be reachable with --host since that
dnl relies on the build environment being real, rather than mocked up.
AC_DEFUN([CF_WITH_SYSTYPE],[
CF_CHECK_CACHE([AC_CANONICAL_SYSTEM])
AC_ARG_WITH(system-type,
	[  --with-system-type=XXX  test: override derived host system-type],
[AC_MSG_WARN(overriding system type to $withval)
	cf_cv_system_name=$withval
	host_os=$withval
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_VALGRIND version: 1 updated: 2006/12/14 18:00:21
dnl ----------------
AC_DEFUN([CF_WITH_VALGRIND],[
CF_NO_LEAKS_OPTION(valgrind,
	[  --with-valgrind         test: use valgrind],
	[USE_VALGRIND])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_ZLIB version: 6 updated: 2024/09/10 19:19:44
dnl ------------
dnl check for libz aka "zlib"
dnl
dnl $1 if present is yes/no, e.g., from a --with-zlib option.
AC_DEFUN([CF_WITH_ZLIB],[
  ifelse([$1],,,[CF_ADD_OPTIONAL_PATH($1, [zlib])])

  CF_FIND_LINKAGE([
#include <zlib.h>
],[
	gzopen("name","mode")
],z,,,zlib)

AC_CHECK_FUNCS( \
	zError \
)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_SOURCE version: 69 updated: 2025/07/26 14:09:49
dnl ---------------
dnl Try to get _XOPEN_SOURCE defined properly that we can use POSIX functions,
dnl or adapt to the vendor's definitions to get equivalent functionality,
dnl without losing the common non-POSIX features.
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
dnl	$2 is the nominal value for _POSIX_C_SOURCE
dnl
dnl The default case prefers _XOPEN_SOURCE over _POSIX_C_SOURCE if the
dnl implementation predefines it, because X/Open and most implementations agree
dnl that the latter is a legacy or "aligned" value.
dnl
dnl Because _XOPEN_SOURCE is preferred, if defining _POSIX_C_SOURCE turns
dnl that off, then refrain from setting _POSIX_C_SOURCE explicitly.
dnl
dnl References:
dnl https://pubs.opengroup.org/onlinepubs/007904975/functions/xsh_chap02_02.html
dnl https://docs.oracle.com/cd/E19253-01/816-5175/standards-5/index.html
dnl https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
AC_DEFUN([CF_XOPEN_SOURCE],[
AC_REQUIRE([AC_CANONICAL_HOST])
AC_REQUIRE([CF_POSIX_VISIBLE])

if test "$cf_cv_posix_visible" = no; then

cf_XOPEN_SOURCE=ifelse([$1],,500,[$1])
cf_POSIX_C_SOURCE=ifelse([$2],,199506L,[$2])
cf_xopen_source=

case "$host_os" in
(aix[[4-7]]*)
	cf_xopen_source="-D_ALL_SOURCE"
	;;
(darwin[[0-8]].*)
	cf_xopen_source="-D_APPLE_C_SOURCE"
	;;
(darwin*)
	cf_xopen_source="-D_DARWIN_C_SOURCE"
	cf_XOPEN_SOURCE=
	;;
(freebsd*|dragonfly*|midnightbsd*)
	# 5.x headers associate
	#	_XOPEN_SOURCE=600 with _POSIX_C_SOURCE=200112L
	#	_XOPEN_SOURCE=500 with _POSIX_C_SOURCE=199506L
	cf_POSIX_C_SOURCE=200112L
	cf_XOPEN_SOURCE=600
	cf_xopen_source="-D_BSD_TYPES -D__BSD_VISIBLE -D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	;;
(hpux11*)
	cf_xopen_source="-D_HPUX_SOURCE -D_XOPEN_SOURCE=500"
	;;
(hpux*)
	cf_xopen_source="-D_HPUX_SOURCE"
	;;
(irix[[56]].*)
	cf_xopen_source="-D_SGI_SOURCE"
	cf_XOPEN_SOURCE=
	;;
(linux*gnu|linux*gnuabi64|linux*gnuabin32|linux*gnuabielfv*|linux*gnueabi|linux*gnueabihf|linux*gnux32|uclinux*|gnu*|mint*|k*bsd*-gnu|cygwin|msys|mingw*|linux*uclibc)
	CF_GNU_SOURCE($cf_XOPEN_SOURCE)
	;;
linux*musl)
	cf_xopen_source="-D_BSD_SOURCE"
	;;
(minix*)
	cf_xopen_source="-D_NETBSD_SOURCE" # POSIX.1-2001 features are ifdef'd with this...
	;;
(mirbsd*)
	# setting _XOPEN_SOURCE or _POSIX_SOURCE breaks <sys/select.h> and other headers which use u_int / u_short types
	cf_XOPEN_SOURCE=
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
(netbsd*)
	cf_xopen_source="-D_NETBSD_SOURCE" # setting _XOPEN_SOURCE breaks IPv6 for lynx on NetBSD 1.6, breaks xterm, is not needed for ncursesw
	;;
(openbsd[[6-9]]*)
	# OpenBSD 6.x has broken locale support, both compile-time and runtime.
	# see https://www.mail-archive.com/bugs@openbsd.org/msg13200.html
	# Abusing the conformance level is a workaround.
	AC_MSG_WARN(this system does not provide usable locale support)
	cf_xopen_source="-D_BSD_SOURCE"
	cf_XOPEN_SOURCE=700
	;;
(openbsd[[4-5]]*)
	# setting _XOPEN_SOURCE lower than 500 breaks g++ compile with wchar.h, needed for ncursesw
	cf_xopen_source="-D_BSD_SOURCE"
	cf_XOPEN_SOURCE=600
	;;
(openbsd*)
	# setting _XOPEN_SOURCE breaks xterm on OpenBSD 2.8, is not needed for ncursesw
	;;
(osf[[45]]*)
	cf_xopen_source="-D_OSF_SOURCE"
	;;
(nto-qnx*)
	cf_xopen_source="-D_QNX_SOURCE"
	;;
(sco*)
	# setting _XOPEN_SOURCE breaks Lynx on SCO Unix / OpenServer
	;;
(solaris2.*)
	cf_xopen_source="-D__EXTENSIONS__"
	cf_cv_xopen_source=broken
	;;
(sysv4.2uw2.*) # Novell/SCO UnixWare 2.x (tested on 2.1.2)
	cf_XOPEN_SOURCE=
	cf_POSIX_C_SOURCE=
	;;
(*)
	CF_TRY_XOPEN_SOURCE
	cf_save_xopen_cppflags="$CPPFLAGS"
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	# Some of these niche implementations use copy/paste, double-check...
	if test "$cf_cv_xopen_source" = no ; then
		CF_VERBOSE(checking if _POSIX_C_SOURCE interferes with _XOPEN_SOURCE)
		AC_TRY_COMPILE(CF__XOPEN_SOURCE_HEAD,CF__XOPEN_SOURCE_BODY,,[
			AC_MSG_WARN(_POSIX_C_SOURCE definition is not usable)
			CPPFLAGS="$cf_save_xopen_cppflags"])
	fi
	;;
esac

if test -n "$cf_xopen_source" ; then
	CF_APPEND_CFLAGS($cf_xopen_source,true)
fi

dnl In anything but the default case, we may have system-specific setting
dnl which is still not guaranteed to provide all of the entrypoints that
dnl _XOPEN_SOURCE would yield.
if test -n "$cf_XOPEN_SOURCE" && test -z "$cf_cv_xopen_source" ; then
	AC_MSG_CHECKING(if _XOPEN_SOURCE really is set)
	AC_TRY_COMPILE([#include <stdlib.h>],[
#ifndef _XOPEN_SOURCE
#error _XOPEN_SOURCE is not defined
#endif],
	[cf_XOPEN_SOURCE_set=yes],
	[cf_XOPEN_SOURCE_set=no])
	AC_MSG_RESULT($cf_XOPEN_SOURCE_set)
	if test "$cf_XOPEN_SOURCE_set" = yes
	then
		AC_TRY_COMPILE([#include <stdlib.h>],[
#if (_XOPEN_SOURCE - 0) < $cf_XOPEN_SOURCE
#error (_XOPEN_SOURCE - 0) < $cf_XOPEN_SOURCE
#endif],
		[cf_XOPEN_SOURCE_set_ok=yes],
		[cf_XOPEN_SOURCE_set_ok=no])
		if test "$cf_XOPEN_SOURCE_set_ok" = no
		then
			AC_MSG_WARN(_XOPEN_SOURCE is lower than requested)
		fi
	else
		CF_TRY_XOPEN_SOURCE
	fi
fi
fi # cf_cv_posix_visible
])
dnl ---------------------------------------------------------------------------
dnl CF_X_FONTENC version: 7 updated: 2011/10/28 18:48:08
dnl ------------
dnl
dnl First check for the appropriate config program, since the developers for
dnl these libraries change their configuration (and config program) more or
dnl less randomly.  If we cannot find the config program, do not bother trying
dnl to guess the latest variation of include/lib directories.
dnl
dnl If the package-config program is not found, rely on the configure script's
dnl options to provide the cflags/libs options:
dnl	--with-fontenc-cflags
dnl	--with-fontenc-libs
AC_DEFUN([CF_X_FONTENC],
[
AC_REQUIRE([CF_WITH_ZLIB])
AC_REQUIRE([CF_PKG_CONFIG])

cf_extra_fontenc_incs=
cf_extra_fontenc_libs="-lfontenc -lX11"
FONTENC_CONFIG=
FONTENC_PARAMS=

if test "$PKG_CONFIG" != none;  then
    if "$PKG_CONFIG" --exists fontenc; then
        FONTENC_CONFIG=$PKG_CONFIG
        FONTENC_PARAMS="fontenc x11"
        cf_extra_fontenc_libs=
    fi
fi

AC_MSG_CHECKING(for optional font-encoding cflags)
AC_ARG_WITH(fontenc-cflags,
	[  --with-fontenc-cflags   -D/-I options for compiling with font encoding library],
    [cf_x_fontenc_incs="$withval"],[cf_x_fontenc_incs=auto])
test "$cf_x_fontenc_incs" = yes && cf_x_fontenc_incs=auto
AC_MSG_RESULT($cf_x_fontenc_incs)

AC_MSG_CHECKING(for optional font-encoding libraries)
AC_ARG_WITH(fontenc-libs,
	[  --with-fontenc-libs     -L/-l options to link font encoding library],
    [cf_x_fontenc_libs="$withval"],[cf_x_fontenc_libs=auto])
test "$cf_x_fontenc_libs" = yes && cf_x_fontenc_libs=auto
AC_MSG_RESULT($cf_x_fontenc_libs)

if test "$cf_x_fontenc_incs" = auto ; then
AC_CACHE_CHECK(for X font-encoding headers,cf_cv_x_fontenc_incs,[
    if test -n "$FONTENC_CONFIG" ; then
        cf_cv_x_fontenc_incs="`$FONTENC_CONFIG $FONTENC_PARAMS --cflags 2>/dev/null`"
    else
        cf_cv_x_fontenc_incs=$cf_extra_fontenc_incs
    fi
])
else
	cf_cv_x_fontenc_incs=$cf_x_fontenc_incs
fi

if test "$cf_x_fontenc_libs" = auto ; then
AC_CACHE_CHECK(for X font-encoding libraries,cf_cv_x_fontenc_libs,[
    if test -n "$FONTENC_CONFIG" ; then
        cf_cv_x_fontenc_libs="`$FONTENC_CONFIG $FONTENC_PARAMS --libs 2>/dev/null`"
    else
        cf_cv_x_fontenc_libs=$cf_extra_fontenc_libs
    fi
])
else
	cf_cv_x_fontenc_libs=$cf_x_fontenc_libs
fi

cf_save_LIBS="$LIBS"
cf_save_INCS="$CPPFLAGS"

LIBS="$cf_cv_x_fontenc_libs $LIBS $X_LIBS"
CPPFLAGS="$CPPFLAGS $X_CFLAGS $cf_cv_x_fontenc_incs"

AC_MSG_CHECKING(if we can link with font encoding library)
AC_TRY_LINK([
$ac_includes_default
#include <X11/Xlib.h>
#include <X11/fonts/fontenc.h>],[
	FontMapPtr mapping = FontEncMapFind(0, FONT_ENCODING_UNICODE, -1, -1, NULL);
	],[cf_have_fontenc_libs=yes],[cf_have_fontenc_libs=no])
AC_MSG_RESULT($cf_have_fontenc_libs)

LIBS="$cf_save_LIBS"
CPPFLAGS="$cf_save_INCS"

if test $cf_have_fontenc_libs = yes ; then
	LIBS="$cf_cv_x_fontenc_libs $LIBS $X_LIBS"
	CF_ADD_CFLAGS($cf_cv_x_fontenc_incs)
else
	AC_MSG_WARN(No libraries found for font-encoding)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF__GRANTPT_BODY version: 6 updated: 2021/06/07 17:39:17
dnl ----------------
dnl Body for workability check of grantpt.
define([CF__GRANTPT_BODY],[
	int code = 0;
	int pty;
	int tty;
	char *slave;
	struct termios tio;

	(void)tio;
	signal(SIGALRM, my_timeout);

	if (alarm(2) == 9)
		failed(9);
	else if ((pty = posix_openpt(O_RDWR)) < 0)
		failed(1);
	else if (grantpt(pty) < 0)
		failed(2);
	else if (unlockpt(pty) < 0)
		failed(3);
	else if ((slave = ptsname(pty)) == 0)
		failed(4);
#if (CONFTEST == 3) || defined(CONFTEST_isatty)
	else if (!isatty(pty))
		failed(4);
#endif
#if CONFTEST >= 4
    else if (tcgetattr(pty, &tio) < 0)
		failed(20);
    else if (tcsetattr(pty, TCSAFLUSH, &tio) < 0)
		failed(21);
#endif
	/* BSD posix_openpt does not treat pty as a terminal until slave is opened.
	 * Linux does treat it that way.
	 */
	else if ((tty = open(slave, O_RDWR)) < 0)
		failed(5);
#ifdef CONFTEST
#ifdef I_PUSH
#if (CONFTEST == 0) || defined(CONFTEST_ptem)
    else if (ioctl(tty, I_PUSH, "ptem") < 0)
		failed(10);
#endif
#if (CONFTEST == 1) || defined(CONFTEST_ldterm)
    else if (ioctl(tty, I_PUSH, "ldterm") < 0)
		failed(11);
#endif
#if (CONFTEST == 2) || defined(CONFTEST_ttcompat)
    else if (ioctl(tty, I_PUSH, "ttcompat") < 0)
		failed(12);
#endif
#endif /* I_PUSH */
#if CONFTEST >= 5
    else if (tcgetattr(tty, &tio) < 0)
		failed(30);
    else if (tcsetattr(tty, TCSAFLUSH, &tio) < 0)
		failed(31);
#endif
#endif /* CONFTEST */
    (void) tty;
	${cf_cv_main_return:-return}(code);
])
dnl ---------------------------------------------------------------------------
dnl CF__GRANTPT_HEAD version: 4 updated: 2020/03/10 18:53:47
dnl ----------------
dnl Headers for workability check of grantpt.
define([CF__GRANTPT_HEAD],[
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#ifndef HAVE_POSIX_OPENPT
#undef posix_openpt
#define posix_openpt(mode) open("/dev/ptmx", mode)
#endif

#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

static void failed(int code)
{
	perror("conftest");
	exit(code);
}

static void my_timeout(int sig)
{
	(void)sig;
	exit(99);
}
])
dnl ---------------------------------------------------------------------------
dnl CF__ICONV_BODY version: 2 updated: 2007/07/26 17:35:47
dnl --------------
dnl Test-code needed for iconv compile-checks
define([CF__ICONV_BODY],[
	iconv_t cd = iconv_open("","");
	iconv(cd,NULL,NULL,NULL,NULL);
	iconv_close(cd);]
)dnl
dnl ---------------------------------------------------------------------------
dnl CF__ICONV_HEAD version: 1 updated: 2007/07/26 15:57:03
dnl --------------
dnl Header-files needed for iconv compile-checks
define([CF__ICONV_HEAD],[
#include <stdlib.h>
#include <iconv.h>]
)dnl
dnl ---------------------------------------------------------------------------
dnl CF__XOPEN_SOURCE_BODY version: 2 updated: 2023/02/18 17:41:25
dnl ---------------------
dnl body of test when test-compiling for _XOPEN_SOURCE check
define([CF__XOPEN_SOURCE_BODY],
[
#ifndef _XOPEN_SOURCE
#error _XOPEN_SOURCE is not defined
#endif
])
dnl ---------------------------------------------------------------------------
dnl CF__XOPEN_SOURCE_HEAD version: 2 updated: 2023/02/18 17:41:25
dnl ---------------------
dnl headers to include when test-compiling for _XOPEN_SOURCE check
define([CF__XOPEN_SOURCE_HEAD],
[
$ac_includes_default
])
