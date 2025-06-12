CONFIG_PAPIEX_DEBUG?=n
CONFIG_PAPIEX_PAPI?=n

ifeq ($(CONFIG_PAPIEX_PAPI),y)
DEFINES+=-DHAVE_PAPI
endif
ifeq ($(CONFIG_PAPIEX_DEBUG),y)
DEFINES+=-DDEBUG
endif

# This needs to be the same as the top-level Makefile!
#
# If the API changed, bump this and reset all
INTERFACE=2
# If API has been added to but has backwards compat, bump this.
AGE=3
# If anything changed at all bump this.
REVISION=15
VERSION=$(INTERFACE).$(AGE).$(REVISION)

LIBNAME=papiex

include Rules.papi
include Rules.monitor
include Rules.gmake
include Rules.export
DEFINES += -DPAPIEX_VERSION=\"$(VERSION)\"

maketest.output: 
	cd $(build-dir)/tests; ./runtests > ../../$@ 2>&1

.PHONY: install-native
install-native:
	$(MAKE) -C $(build-dir) -f $(root-dir)/src/Makefile install

.PHONY: uninstall-native
uninstall-native:
	$(MAKE) -C $(build-dir) -f $(root-dir)/src/Makefile uninstall

.PHONY: www-native
www-native: 
	-mkdir www/samples
	cp samples/sample.* samples/output.sample www/samples
