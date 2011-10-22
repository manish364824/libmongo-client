LMC_CURRENT			= 4
LMC_REVISION			= 0
LMC_AGE				= 0

BEEJR_CURRENT			= 0
BEEJR_REVISION			= 0
BEEJR_AGE			= 0

if WITH_SYMVER
LMC_SYMVER			= -Wl,--version-script,${srcdir}/libmongo-client.ver
BEEJR_SYMVER			= -Wl,--version-script,${srcdir}/libbeejr.ver
else
LMC_SYMVER			=
BEEJR_SYMBER			=
endif

lib_LTLIBRARIES			= libbeejr.la libmongo-client.la
libmongo_client_la_LDFLAGS	= -version-info ${LMC_CURRENT}:${LMC_REVISION}:${LMC_AGE} ${LMC_SYMVER}
libmongo_client_la_LIBADD	= libbeejr.la
libbeejr_la_LDFLAGS		= -version-info ${BEEJR_CURRENT}:${BEEJR_REVISION}:${BEEJR_AGE} ${BEEJR_SYMVER}

noinst_LTLIBRARIES		= libmongo-client-internal.la libbeejr-internal.la

libmongo_client_la_SOURCES	= \
	${commonHEADERS}

libbeejr_la_SOURCES	= \
	${commonHEADERS}	  \
	lmc/bson-element.h	  \
	\
	bson-element.c

libmongo_client_internal_la_SOURCES	= ${libmongo_client_la_SOURCES}
libmongo_client_internal_la_LIBADD	= libbeejr-internal.la
libbeejr_internal_la_SOURCES		= ${libbeejr_la_SOURCES}

commonHEADERS			= \
	lmc.h			  \
	lmc/common.h		  \
	lmc/endian.h

includedir			= $(prefix)/include/mongo-client
nobase_include_HEADERS		= \
	${commonHEADERS}	  \
	\
	lmc/bson-element.h

pkgconfigdir			= $(libdir)/pkgconfig
pkgconfig_DATA			= libbeejr.pc libmongo-client.pc

CLEANFILES			= *.gcda *.gcno *.gcov
CLEANDIRS			= coverage

coverage:
	$(AM_V_GEN)
	$(AM_V_at) SOURCES="$(SOURCES)" builddir="$(builddir)" srcdir="$(srcdir)" top_srcdir="$(top_srcdir)" $(top_srcdir)/t/tools/coverage.sh

clean-local:
	-test -z "$(CLEANDIRS)" || rm -rf "$(CLEANDIRS)"

.PHONY: coverage
