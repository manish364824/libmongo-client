LMC_CURRENT			= 4
LMC_REVISION			= 0
LMC_AGE				= 0

lib_LTLIBRARIES			= libmongo-client.la
libmongo_client_la_LDFLAGS	= -version-info ${LMC_CURRENT}:${LMC_REVISION}:${LMC_AGE} @vsymldflags@

libmongo_client_la_SOURCES	= 		\
	${libmongo_client_include_HEADERS}	\
	lmc/private.h				\
	\
	bson.c

libmongo_client_includedir	= $(includedir)/mongo-client
libmongo_client_include_HEADERS	= \
	lmc/common.h lmc/error.h lmc/bson.h

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
