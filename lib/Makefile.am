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

libbeejr_la_SOURCES	=	  \
	${commonHEADERS}	  \
	lmc/bson-element.h	  \
	lmc/bson.h		  \
	\
	bson-element.c		  \
	bson.c

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
	lmc/bson.h		  \
	lmc/bson-element.h

pkgconfigdir			= $(libdir)/pkgconfig
pkgconfig_DATA			= libbeejr.pc libmongo-client.pc

CLEANFILES			= *.gcda *.gcno *.gcov \
				${stringish_SETTERS} ${simple_SETTERS}
CLEANDIRS			= coverage

# Setter factory magic!
stringish_SETTERS		= bson-element-string.c bson-element-js-code.c
simple_SETTERS			= bson-element-double.c bson-element-int32.c \
				bson-element-int64.c bson-element-datetime.c \
				bson-element-boolean.c

bson-element.c: ${stringish_SETTERS} ${simple_SETTERS}

bson-element-string.c: SETTER=string
bson-element-string.c: TYPE=STRING
bson-element-js-code.c: SETTER=javascript
bson-element-js-code.c: TYPE=JS_CODE
${stringish_SETTERS}: bson-element-stringish.c.in
	$(AM_V_GEN)
	$(AM_V_at) sed -e "s/SETTER/${SETTER}/g" \
		       -e "s/_TYPE/_${TYPE}/g" \
		       -e "s/TYPE/BSON_TYPE_${TYPE}/g" \
		       <$^ >$@

bson-element-double.c: CTYPE=double
bson-element-double.c: ACCESSOR=dbl

bson-element-int32.c: CTYPE=int32_t
bson-element-int32.c: ACCESSOR=i32

bson-element-int64.c: CTYPE=int64_t
bson-element-int64.c: ACCESSOR=i64

bson-element-datetime.c: CTYPE=int64_t
bson-element-datetime.c: SETTER=datetime
bson-element-datetime.c: TYPE=UTC_DATETIME
bson-element-datetime.c: ACCESSOR=i64

bson-element-boolean.c: CTYPE=lmc_bool_t
bson-element-boolean.c: SETTER=boolean
bson-element-boolean.c: ACCESSOR=bool
bson-element-boolean.c: SIZE=1
bson-element-boolean.c: NOSWAP=1

${simple_SETTERS}: SIZE ?= sizeof (${CTYPE})
${simple_SETTERS}: TYPE ?= $(shell echo ${SETTER} | tr '[:lower:]' '[:upper:]')
${simple_SETTERS}: SETTER ?= $(shell echo ${CTYPE} | sed -e 's,_t$$,,')
${simple_SETTERS}: SWAPPER = $(shell echo ${CTYPE} | sed -e 's,_t$$,,' | tr '[:lower:]' '[:upper:]')

${simple_SETTERS}: bson-element-simple.c.in
	$(AM_V_GEN)
	$(AM_V_at) sed -e "s/SETTER/${SETTER}/g" \
		       -e "s/LMC_CTYPE_/LMC_${SWAPPER}_/g" \
		       -e "s/CTYPE/${CTYPE}/g" \
		       -e "s/_TYPE/_${TYPE}/g" \
		       -e "s/TYPE/BSON_TYPE_${TYPE}/g" \
		       -e "s/ACCESSOR/${ACCESSOR}/g" \
		       -e "s/SIZE/${SIZE}/g" \
		       <$^ >$@.tmp
	$(AM_V_at) if [ ! -z "${NOSWAP}" ]; then \
			sed -e "s,LMC_.*LE,,g" <$@.tmp >$@; \
			rm $@.tmp; \
		   else \
			mv $@.tmp $@; \
		   fi

coverage:
	$(AM_V_GEN)
	$(AM_V_at) SOURCES="$(SOURCES)" builddir="$(builddir)" srcdir="$(srcdir)" top_srcdir="$(top_srcdir)" $(top_srcdir)/t/tools/coverage.sh

clean-local:
	-test -z "$(CLEANDIRS)" || rm -rf "$(CLEANDIRS)"

.PHONY: coverage
