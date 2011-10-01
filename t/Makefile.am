SUBDIRS			= libtap

bson_unit_tests		= \
		unit/bson/bson_new \
		unit/bson/bson_reset \
		unit/bson/bson_new_from_data \
		\
		unit/bson/bson_type_as_string \
		unit/bson/bson_validate_key \
		\
		unit/bson/bson_append_string \
		unit/bson/bson_append_double \
		unit/bson/bson_append_document \
		unit/bson/bson_append_array \
		unit/bson/bson_append_binary \
		unit/bson/bson_append_int32 \
		unit/bson/bson_append_oid \
		unit/bson/bson_append_boolean \
		unit/bson/bson_append_utc_datetime \
		unit/bson/bson_append_null \
		unit/bson/bson_append_regexp \
		unit/bson/bson_append_js_code \
		unit/bson/bson_append_symbol \
		unit/bson/bson_append_js_code_w_scope \
		unit/bson/bson_append_timestamp \
		unit/bson/bson_append_int64 \
		\
		unit/bson/bson_cursor_new \
		\
		unit/bson/bson_cursor_find \
		unit/bson/bson_cursor_next \
		unit/bson/bson_cursor_type \
		unit/bson/bson_cursor_type_as_string \
		unit/bson/bson_cursor_key \
		\
		unit/bson/bson_cursor_get_array \
		unit/bson/bson_cursor_get_binary \
		unit/bson/bson_cursor_get_boolean \
		unit/bson/bson_cursor_get_document \
		unit/bson/bson_cursor_get_double \
		unit/bson/bson_cursor_get_int32 \
		unit/bson/bson_cursor_get_int64 \
		unit/bson/bson_cursor_get_javascript \
		unit/bson/bson_cursor_get_javascript_w_scope \
		unit/bson/bson_cursor_get_oid \
		unit/bson/bson_cursor_get_regex \
		unit/bson/bson_cursor_get_string \
		unit/bson/bson_cursor_get_symbol \
		unit/bson/bson_cursor_get_timestamp \
		unit/bson/bson_cursor_get_utc_datetime

bson_func_tests		= \
		func/bson/f_bson_empty \
		func/bson/f_bson_chained \
		func/bson/f_bson_weird_types \
		func/bson/f_bson_huge_doc

UNIT_TESTS	= ${bson_unit_tests}
FUNC_TESTS	= ${bson_func_tests}
PERF_TESTS	= ${bson_perf_tests}
TESTCASES	= ${UNIT_TESTS} ${FUNC_TESTS} ${PERF_TESTS}

check_PROGRAMS	= ${TESTCASES}
noinst_PROGRAMS= test_cleanup

AM_CFLAGS = -I$(top_srcdir)/lib/ -I${top_srcdir}/t/libtap/
LDADD = $(top_builddir)/lib/libmongo-client.la ${top_builddir}/t/libtap/libtap.la

EXTRA_DIST = README \
	     coverage.sh \
             tools/coverage-report-entry.pl tools/coverage-report.pl \
             tools/coverage-report.xsl

check-%: BASE=$(subst -,_,$(subst check-,,$@))
check-%: TESTCASES=$(value $(BASE)_unit_tests) $(value $(BASE)_func_tests) $(value $(BASE)_tests)
check-%: ${TESTCASES} ${noinst_PROGRAMS}
	$(AM_V_at) ${builddir}/test_cleanup
	$(AM_V_GEN) srcdir=${srcdir} prove -e "${PROVE_ENV}" ${PROVE_OPTIONS} ${TESTCASES}
	$(AM_V_at) ${builddir}/test_cleanup

check: ${TESTCASES} ${noinst_PROGRAMS}
	$(AM_V_at) ${builddir}/test_cleanup
	$(AM_V_GEN) srcdir=${srcdir} prove -e "${PROVE_ENV}" ${PROVE_OPTIONS} ${TESTCASES}
	$(AM_V_at) ${builddir}/test_cleanup

.PHONY: check
