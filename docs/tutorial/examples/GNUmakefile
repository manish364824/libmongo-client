# NOTE: This Makefile assumes that a recent enough version of
# libmongo-client is installed!
#
# It will NOT work in the build directory, without an installed
# libmongo-client library.

TUTORIAL_PROGRAMS	= tut/bson_build \
			  tut/bson_traverse \
			  tut/mongo_sync \
			  tut/hl_client

LMC_CFLAGS		= $(shell pkg-config --cflags libmongo-client)
LMC_LIBS		= $(shell pkg-config --libs libmongo-client)

all: ${TUTORIAL_PROGRAMS}
clean:
	rm -f ${TUTORIAL_PROGRAMS}
	-rmdir tut/

${TUTORIAL_PROGRAMS}: tut/%: tut_%.c
	@install -d tut
	${CC} ${LMC_CFLAGS} ${CFLAGS} ${LMC_LIBS} ${LDFLAGS} $< -o $@

.PHONY: all clean
