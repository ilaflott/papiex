OS_TARGET?=centos-7
# Please sync with papiex/Makefile
VERSION=2.3.14
# Please sync with papiex/Makefile
RELEASE=papiex-epmt-$(VERSION)-$(OS_TARGET).tgz
#
DEFINES=-DHAVE_MONITOR -DHAVE_PAPI
#
SHELL = /bin/bash
CC := gcc
OCC := $(CC)
PREFIX := $(shell pwd)/papiex-epmt-install
LIBMONITOR := $(DESTDIR)$(PREFIX)/lib/libmonitor.so
LIBPFM := $(DESTDIR)$(PREFIX)/lib/libpfm.so
LIBPAPI := $(DESTDIR)$(PREFIX)/lib/libpapi.so
LIBPAPIEX := $(DESTDIR)$(PREFIX)/lib/libpapiex.so
DEPS = $(LIBMONITOR) $(LIBPFM) $(LIBPAPI) $(LIBPAPIEX)
#
ifneq (,$(findstring HAVE_PAPI,$(DEFINES)))
PAPI_PREFIX := $(PREFIX)
PAPI_INC_PATH ?= $(PAPI_PREFIX)/include
PAPI_LIB_PATH ?= $(PAPI_PREFIX)/lib
endif
#
MONITOR_PREFIX := $(PREFIX)
MONITOR_INC_PATH ?= $(MONITOR_PREFIX)/include
MONITOR_LIB_PATH ?= $(MONITOR_PREFIX)/lib
#
ifneq (,$(findstring HAVE_PAPI,$(DEFINES)))
PAPI_CONFIGURE_ARGS = --with-static-user-events --with-static-papi-events --enable-perfevent_rdpmc=no --disable-perf_event_uncore --prefix=$(PAPI_PREFIX) --with-pfm-root=$(shell pwd)/libpfm --with-debug=no
endif

export PREFIX CC OCC SHELL DEFINES

.PHONY: monitor papiex papi check libpfm install dist
.PHONY: docker-clean docker-distclean docker-dist docker-check docker-test-dist

install: papiex

dist: $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/lib
	mv $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/bin.old
	mkdir -p $(DESTDIR)$(PREFIX)/bin
ifneq (,$(findstring HAVE_PAPI,$(DEFINES)))
	for f in papi_command_line papi_native_avail papi_avail papi_component_avail check_events showevtinfo ; do strip $(DESTDIR)$(PREFIX)/bin.old/$$f; cp $(DESTDIR)$(PREFIX)/bin.old/$$f $(DESTDIR)$(PREFIX)/bin/$$f; done
endif
	for f in monitor-link monitor-run; do cp $(DESTDIR)$(PREFIX)/bin.old/$$f $(DESTDIR)$(PREFIX)/bin/$$f; done
	rm -f $(DESTDIR)$(PREFIX)/lib/libmonitor.la
	for f in $(DESTDIR)$(PREFIX)/lib/*; do strip $$f; done
	rm -r $(DESTDIR)$(PREFIX)/bin.old
	rm -rf $(DESTDIR)$(PREFIX)/include
	rm -rf $(DESTDIR)$(PREFIX)/share
	cd $(DESTDIR)$(PREFIX)/..; rm -f $(RELEASE); tar cvfz $(RELEASE) `basename $(PREFIX)`

dist-test:
	cd papiex/src; tar cvfz ../../test-$(RELEASE) tests

check: 
	cd papiex; $(MAKE) PREFIX=$(PREFIX) LIBPAPIEX=$(LIBPAPIEX) check

ifneq (,$(findstring HAVE_PAPI,$(DEFINES)))
papiex $(LIBPAPIEX): papi monitor
else
papiex $(LIBPAPIEX): monitor
endif
	cd papiex; $(MAKE) CC=$(CC) OCC=$(OCC) MONITOR_INC_PATH=$(MONITOR_INC_PATH) MONITOR_LIB_PATH=$(MONITOR_LIB_PATH) PAPI_INC_PATH=$(PAPI_INC_PATH) PAPI_LIB_PATH=$(PAPI_LIB_PATH) install

monitor/Makefile:
	cd monitor; ./configure --prefix=$(MONITOR_PREFIX)
monitor $(LIBMONITOR): monitor/Makefile
	cd monitor; $(MAKE) install

ifneq (,$(findstring HAVE_PAPI,$(DEFINES)))
papi/src/Makefile:
	cd papi/src ; ./configure $(PAPI_CONFIGURE_ARGS)
papi $(LIBPAPI): libpfm papi/src/Makefile
	$(MAKE) -C papi/src install-lib install-utils

libpfm $(LIBPFM):
	$(MAKE) -C libpfm OPTIM=-O2 LDCONFIG=true install-lib 
	$(MAKE) -C libpfm/examples OPTIM=-O2 EXAMPLESDIR=$(DESTDIR)$(PREFIX)/bin install-examples 
endif

.PHONY: clean
clean:
	-cd monitor; $(MAKE) clean
	-cd libpfm; $(MAKE) clean
ifneq (,$(findstring HAVE_PAPI,$(DEFINES)))
	-cd papi/src; $(MAKE) clean
	-cd papiex; $(MAKE) clean
endif
	-rm -f *~

.PHONY: distclean clobber
distclean clobber: clean
	-cd monitor; $(MAKE) distclean
ifneq (,$(findstring HAVE_PAPI,$(DEFINES)))
	-cd libpfm; $(MAKE) distclean
	-cd papi/src; $(MAKE) distclean
endif
	-cd papiex; $(MAKE) distclean
	rm -rf papiex-epmt-install test-$(RELEASE) $(RELEASE)

# if on m-chip mac... use this
DOCKER_BUILD:=docker build --platform linux/x86_64 -f
#DOCKER_BUILD:=docker build -f

#
# Docker targets
#

# This target isn't optimal as it mounts the $(PWD) to the docker image,
# meaning the source could be contaminated, but the distclean target should
# ensure everything is cleaned up. Also, the formal release target should check
# for any diffs against the repo and complain.
$(RELEASE) test-$(RELEASE) docker-dist:
	$(DOCKER_BUILD) Dockerfiles/Dockerfile.$(OS_TARGET)-papiex-build -t $(OS_TARGET)-papiex-build .
	docker run --privileged --rm -it -v `pwd`:/build -w /build $(OS_TARGET)-papiex-build make OS_TARGET=$(OS_TARGET) distclean install dist dist-test

docker-test-dist: $(RELEASE) test-$(RELEASE)
	$(DOCKER_BUILD) Dockerfiles/Dockerfile.$(OS_TARGET)-papiex-test -t $(OS_TARGET)-papiex-test --build-arg release=$(RELEASE) .
	docker run --privileged --rm -it $(OS_TARGET)-papiex-test

docker-check:
	$(DOCKER_BUILD) Dockerfiles/Dockerfile.$(OS_TARGET)-papiex-test -t $(OS_TARGET)-papiex-test --build-arg release=$(RELEASE) .
	docker run --privileged --rm -it -v `pwd`:/build -w /build $(OS_TARGET)-papiex-test /tmp/init.sh make OS_TARGET=$(OS_TARGET) check

docker-clean:
	docker run --rm -it -v `pwd`:/build -w /build $(OS_TARGET)-papiex-build make OS_TARGET=$(OS_TARGET) clean 

docker-distclean:
	docker run --rm -it -v `pwd`:/build -w /build $(OS_TARGET)-papiex-build make OS_TARGET=$(OS_TARGET) distclean 



