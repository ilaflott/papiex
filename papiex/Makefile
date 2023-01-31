DEFINES:=-DHAVE_MONITOR #-DHAVE_PAPI 
LIBNAME=papiex

# This needs to be the same as the top-level Makefile!
#
# If the API changed, bump this and reset all
INTERFACE=2
# If API has been added to but has backwards compat, bump this.
AGE=3
# If anything changed at all bump this.
REVISION=15

VERSION=$(INTERFACE).$(AGE).$(REVISION)

# Put USE_MPI test first as it will change the 
# the compilers to MPICC.
MANPAGES1+=papiex-config.1 papiex.1
MANPAGES3+=papiex_start.3 papiex_stop.3 papiex_accum.3
HTMLMANPAGES1=$(patsubst %.1,%-man.html,$(MANPAGES1))
HTMLMANPAGES3=$(patsubst %.3,%-man.html,$(MANPAGES3))

include Rules.papi
include Rules.monitor
include Rules.gmake
include Rules.export
DEFINES += -DPAPIEX_VERSION=\"$(VERSION)\"

ifneq (,$(MAILBUGS))
DEFINES += -DMAILBUGS=\"$(MAILBUGS)\"
endif

ifneq (,$(MMPERFTOOLS_VERSION))
DEFINES += -DMMPERFTOOLS_VERSION=\"$(MMPERFTOOLS_VERSION)\"
endif

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
