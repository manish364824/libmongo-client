UNIT_TESTS	=
FUNC_TESTS	=
PERF_TESTS	=
TESTCASES	= ${UNIT_TESTS} ${FUNC_TESTS} ${PERF_TESTS}

AM_CFLAGS	= -I$(top_srcdir)/lib/
LDADD		= $(top_builddir)/lib/libmongo-client-internal.la

EXTRA_DIST	= \
		coverage.sh \
		tools/coverage-report-entry.pl tools/coverage-report.pl \
		tools/coverage-report.xsl

check-%: BASE=$(subst -,_,$(subst check-,,$@))
check-%: TESTCASES=$(value $(BASE)_unit_tests) $(value $(BASE)_func_tests) $(value $(BASE)_tests)
check-%: ${TESTCASES} ${noinst_PROGRAMS}
# Run the appropriate tests here

check: ${TESTCASES} ${noinst_PROGRAMS}
# Run the appropriate tests here

.PHONY: check
