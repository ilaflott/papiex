
CC := gcc
OCC := $(CC)
export CC OCC

PREFIX := $(PWD)/papiex-oss-install
DESTPREF := $(PREFIX)

LIBMONITOR := $(DESTPREF)/lib/libmonitor.so
LIBPFM := $(PWD)/lib/libpfm.a
LIBPAPI := $(DESTPREF)/lib/libpapi.so

DEPS =
ifeq (,$(MONITOR_INC_PATH))
  ifeq (,$(MONITOR_LIB_PATH))
    DEPS += $(LIBMONITOR)
  endif
endif

ifeq (,$(PAPI_INC_PATH))
  ifeq (,$(PAPI_LIB_PATH))
    DEPS += $(LIBPFM) $(LIBPAPI)
  endif
endif

PAPI_PREFIX := $(DESTPREF)
PAPI_INC_PATH ?= $(PAPI_PREFIX)/include
PAPI_LIB_PATH ?= $(PAPI_PREFIX)/lib
PAPI_CONFIGURE_ARGS = --with-debug --disable-perf_event_uncore --prefix=$(DESTPREF) --with-pfm-root=$(PWD)/libpfm

MONITOR_PREFIX := $(DESTPREF)
MONITOR_INC_PATH ?= $(MONITOR_PREFIX)/include
MONITOR_LIB_PATH ?= $(MONITOR_PREFIX)/lib



install: install-papiex post-install

# disabled PROFILING_SUPPORT
install-papiex: $(DEPS)
	cd papiex; $(MAKE) CC=$(CC) OCC=$(OCC) FULL_CALIPER_DATA=1 MONITOR_INC_PATH=$(MONITOR_INC_PATH) MONITOR_LIB_PATH=$(MONITOR_LIB_PATH) PAPI_INC_PATH=$(PAPI_INC_PATH) PAPI_LIB_PATH=$(PAPI_LIB_PATH) PREFIX=$(DESTPREF) install

.PHONY: install-monitor
$(LIBMONITOR) install-monitor:
	cd monitor; ./configure --prefix=$(DESTPREF)
	cd monitor; $(MAKE) PREFIX=$(DESTPREF) install

.PHONY: install-papi
$(LIBPAPI) install-papi:
	cd papi/src ; ./configure $(PAPI_CONFIGURE_ARGS)
	$(MAKE) -C papi/src all install install-man

.PHONY: install-libpfm
$(LIBPFM) install-libpfm:
	$(MAKE) -C libpfm PREFIX=$(DESTPREF) LDCONFIG=true all
	$(MAKE) -C libpfm PREFIX=$(DESTPREF) EXAMPLESDIR=$(DESTPREF)/bin LDCONFIG=true install
	$(MAKE) -C libpfm/examples EXAMPLESDIR=$(DESTPREF)/bin LDCONFIG=true install_examples 

.PHONY: clean
clean:
	@if [ -d papi ]; then cd papi/src; [ ! -f Makefile ] || make clean; fi
	@if [ -d monitor ]; then cd monitor; [ ! -f Makefile ] || make clean; fi
	$(MAKE) -C libpfm clean 

.PHONY: distclean clobber
distclean clobber: clean
	@rm -rf papiex/x86_64-Linux
	@if [ -d papi ]; then rm -f papi/src/Makefile papi/src/papi.pc; fi
	@if [ -d monitor ]; then cd monitor; [ ! -f Makefile ] || make distclean; fi
	@if [ -d monitor ]; then cd monitor; [ ! -x configure ] || rm -fv Makefile; fi
	@$(MAKE) -C libpfm distclean
	@rm -rf papiex-oss-install

.PHONY: post-install
post-install:
	cp -a setup/papiex-oss.sh.in $(DESTPREF)/papiex-oss.sh
	cp -a setup/papiex-oss.csh.in $(DESTPREF)/papiex-oss.csh
	cp -a setup/papiex-oss.module.in $(DESTPREF)/papiex-oss
	@echo =======================================================================
	@echo "Tools are installed in:"
	@echo $(DESTPREF)
	@echo
	@echo "To use the tools"
	@echo "----------------"
	@echo "module load $(DESTPREF)/papiex-oss"
	@echo "	   - or -"
	@echo "source $(DESTPREF)/papiex-oss.sh"
	@echo "	   - or -"
	@echo "source $(DESTPREF)/papiex-oss.csh"
	@echo
	@echo "To test install:"
	@echo "make test"
	@echo "make fulltest"
	@echo =======================================================================
	@echo

.PHONY: test
test:
	bash -c 'which papiex 2>/dev/null || source $(DESTPREF)/papiex-oss.sh; cd papiex; make quicktest'

.PHONY: fulltest
fulltest:
	bash -c 'which papiex 2>/dev/null || source $(DESTPREF)/papiex-oss.sh; cd papiex; make test'
