UNIT_TESTS	=
FUNC_TESTS	=
PERF_TESTS	=

TESTS		= ${UNIT_TESTS} ${FUNC_TESTS} ${PERF_TESTS}
check_PROGRAMS	= ${TESTS}

AM_CFLAGS	= -I$(top_srcdir)/lib/ @CHECK_CFLAGS@
LDADD		= $(top_builddir)/lib/libmongo-client-internal.la @CHECK_LIBS@

EXTRA_DIST	= \
		tools/coverage.sh \
		tools/coverage-report-entry.pl tools/coverage-report.pl \
		tools/coverage-report.xsl
