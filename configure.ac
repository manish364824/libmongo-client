dnl Process this file with autoconf to produce a configure script.
dnl

AC_INIT(libmongo-client, 0.2.0, algernon@madhouse-project.org, libmongo-client, https://github.com/algernon/libmongo-client)
AM_INIT_AUTOMAKE([1.9 tar-ustar])

dnl ***************************************************************************
dnl dependencies

dnl ***************************************************************************
dnl Initial setup

AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_CONFIG_HEADER(config.h)

AC_ARG_WITH([versioned-symbols], AC_HELP_STRING([--with-versioned-symbols],[Use versioned symbols]),[dnl
vsymldflags="-Wl,--version-script,\${srcdir}/libmongo-client.ver -Wl,-O1"
],[vsymldflags=])

dnl ***************************************************************************
dnl Checks for programs.
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG
LT_INIT([shared])

dnl ***************************************************************************
dnl Miscellanneous headers
dnl ***************************************************************************

AC_HEADER_STDC

dnl ***************************************************************************
dnl Header checks
dnl ***************************************************************************
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h sys/socket.h netdb.h])

AC_CACHE_CHECK(for struct sockaddr_storage, blb_cv_c_struct_sockaddr_storage,
  [AC_EGREP_HEADER([sockaddr_storage], sys/socket.h, blb_cv_c_struct_sockaddr_storage=yes,blb_cv_c_struct_sockaddr_storage=no)])

if test "$blb_cv_c_struct_sockaddr_storage" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE,[1],[struct sockaddr_storage is present on your system])
fi

AC_CACHE_CHECK(for struct sockaddr_in6, blb_cv_c_struct_sockaddr_in6,
  [AC_EGREP_HEADER([sockaddr_in6], netinet/in.h, blb_cv_c_struct_sockaddr_in6=yes,blb_cv_c_struct_sockaddr_in6=no)])

dnl ***************************************************************************
dnl Checks for libraries
AC_CHECK_FUNC(socket,,
 AC_CHECK_LIB(socket, socket))

AC_FUNC_MMAP
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_FUNCS(memset socket getaddrinfo munmap strtol strerror)

dnl ***************************************************************************
dnl misc features to be enabled
dnl ***************************************************************************
AC_C_INLINE

AC_MSG_CHECKING(whether to enable IPv6 support)
if test "x$blb_cv_c_struct_sockaddr_in6" = "xyes"; then
	enable_ipv6=yes
	AC_MSG_RESULT(yes)
else
	enable_ipv6=no
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING([whether to enable symbol versioning])
AC_SUBST(vsymldflags)
if test -z "${vsymldflags}"; then
   AC_MSG_RESULT([no])
else
   AC_MSG_RESULT([yes, ${vsymldflags}])
fi

AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [package name])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [version number])

AC_OUTPUT(
	Doxyfile
	Makefile
	docs/Makefile
	docs/tutorial/Makefile
	examples/Makefile
	lib/Makefile
	lib/libmongo-client.pc
	t/Makefile
	t/libtap/Makefile
)
