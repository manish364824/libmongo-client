LMC_CURRENT			= 4
LMC_REVISION			= 0
LMC_AGE				= 0

lib_LTLIBRARIES			= libmongo-client.la
libmongo_client_la_LDFLAGS	= -version-info ${LMC_CURRENT}:${LMC_REVISION}:${LMC_AGE} @vsymldflags@

noinst_LTLIBRARIES		= libmongo-client-internal.la

libmongo_client_la_SOURCES	= 		\
	${nobase_include_HEADERS}

libmongo_client_internal_la_SOURCES	= ${libmongo_client_la_SOURCES}

includedir			= $(prefix)/include/mongo-client
nobase_include_HEADERS		=

pkgconfigdir			= $(libdir)/pkgconfig
pkgconfig_DATA			= libmongo-client.pc

CLEANFILES			= *.gcda *.gcno *.gcov
CLEANDIRS			= coverage

coverage:
	$(AM_V_GEN)
	$(AM_V_at) SOURCES="$(SOURCES)" builddir="$(builddir)" srcdir="$(srcdir)" top_srcdir="$(top_srcdir)" $(top_srcdir)/t/coverage.sh

clean-local:
	-test -z "$(CLEANDIRS)" || rm -rf "$(CLEANDIRS)"

.PHONY: coverage
