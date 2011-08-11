if ENABLE_TESTS
test_SUBDIRS	= tests
endif

SUBDIRS		= docs src examples ${test_SUBDIRS}
DIST_SUBDIRS	= ${SUBDIRS}

ACLOCAL_AMFLAGS	= -I m4 --install
EXTRA_DIST	= NEWS README.rst

coverage:
	@echo "Making $@ in src"
	($(am__cd) src && $(MAKE) $(AM_MAKEFLAGS) $@)

doxygen:
	$(AM_V_GEN)
	doxygen
