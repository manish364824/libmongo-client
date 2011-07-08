LMC_CURRENT			= 2
LMC_REVISION			= 0
LMC_AGE				= 2

lib_LTLIBRARIES			= libmongo-client.la
libmongo_client_la_LIBADD	= @GLIB_LIBS@
libmongo_client_la_CFLAGS	= @GLIB_CFLAGS@
libmongo_client_la_LDFLAGS	= -version-info ${LMC_CURRENT}:${LMC_REVISION}:${LMC_AGE} @vsymldflags@

libmongo_client_la_SOURCES	= \
	bson.c bson.h \
	mongo-wire.c mongo-wire.h \
	mongo-client.c mongo-client.h \
	mongo-utils.c mongo-utils.h \
	mongo-sync.c mongo-sync.h \
	mongo-sync-cursor.c mongo-sync-cursor.h \
	mongo-sync-pool.c mongo-sync-pool.h \
	sync-gridfs.c sync-gridfs.h \
	sync-gridfs-chunk.c sync-gridfs-chunk.h \
	sync-gridfs-stream.c sync-gridfs-stream.h \
	mongo.h \
	libmongo-private.h libmongo-macros.h

libmongo_client_includedir	= $(includedir)/mongo-client
libmongo_client_include_HEADERS	= \
	bson.h mongo-wire.h mongo-client.h mongo-utils.h \
	mongo-sync.h mongo-sync-cursor.h mongo-sync-pool.h \
	sync-gridfs.h sync-gridfs-chunk.h sync-gridfs-stream.h \
	mongo.h

pkgconfigdir			= $(libdir)/pkgconfig
pkgconfig_DATA			= libmongo-client.pc

CLEANFILES			= coverage *.gcda *.gcno *.gcov

EXTRA_DIST			= coverage.sh \
				tools/coverage-report-entry.pl tools/coverage-report.pl \
				tools/coverage-report.xsl

coverage:
	$(AM_V_GEN)
	$(AM_V_at) SOURCES="$(SOURCES)" builddir="$(builddir)" srcdir="$(srcdir)" top_srcdir="$(top_srcdir)" $(top_srcdir)/tests/coverage.sh

.PHONY: coverage
