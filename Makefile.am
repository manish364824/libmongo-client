SUBDIRS		= doc lib t

ACLOCAL_AMFLAGS	= -I m4 --install
EXTRA_DIST	= NEWS README.rst

coverage: check
	@echo "Making $@ in lib"
	($(am__cd) lib && $(MAKE) $(AM_MAKEFLAGS) $@)

doxygen:
	$(AM_V_GEN)
	doxygen
