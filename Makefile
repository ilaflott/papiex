
CC := gcc
OCC := $(CC)
export CC OCC

VERSION := 1.0.1
PREFIX := $(PWD)/tools-oss-$(VERSION)
DESTPREF := $(PREFIX)

PAPI_PREFIX := $(DESTPREF)
PAPI_INC_PATH := $(PAPI_PREFIX)/include
PAPI_LIB_PATH := $(PAPI_PREFIX)/lib
PAPI_CONFIGURE_ARGS = --with-debug --disable-perf_event_uncore --prefix=$(DESTPREF) --with-pfm-root=$(PWD)/libpfm

MONITOR_PREFIX := $(DESTPREF)
MONITOR_INC_PATH := $(MONITOR_PREFIX)/include
MONITOR_LIB_PATH := $(MONITOR_PREFIX)/lib


install: install-papiex post-install

install-papiex: $(MONITOR_LIB_PATH)/libmonitor.so $(PAPI_LIB_PATH)/libpapi.so
	cd papiex; $(MAKE) CC=$(CC) OCC=$(OCC) PROFILING_SUPPORT=1 FULL_CALIPER_DATA=1 MONITOR_INC_PATH=$(MONITOR_INC_PATH) PAPI_INC_PATH=$(DESTPREF)/include PAPI_LIB_PATH=$(DESTPREF)/lib PREFIX=$(DESTPREF) install

$(MONITOR_LIB_PATH)/libmonitor.so: 
	cd monitor; ./configure --prefix=$(DESTPREF)
	cd monitor; $(MAKE) PREFIX=$(DESTPREF) install

$(PAPI_LIB_PATH)/libpapi.so: $(DESTPREF)/lib/libpfm.so
	cd papi/src ; ./configure $(PAPI_CONFIGURE_ARGS)
	$(MAKE) -C papi/src all install install-man

$(DESTPREF)/lib/libpfm.so:
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
	@if [ -d papi ]; then rm -f papi/src/Makefile; fi
	@if [ -d monitor ]; then cd monitor; [ ! -f Makefile ] || make distclean; fi
	@if [ -d monitor ]; then cd monitor; [ ! -x configure ] || rm -fv Makefile; fi
	$(MAKE) -C libpfm distclean 

.PHONY: post-install
post-install:
	cp -a setup/tools-oss.sh.in $(DESTPREF)/tools-oss.sh
	cp -a setup/tools-oss.csh.in $(DESTPREF)/tools-oss.csh
	cp -a setup/tools-oss.module.in $(DESTPREF)/tools-oss
	@echo =======================================================================
	@echo "Tools are installed in:"
	@echo $(DESTPREF)
	@echo
	@echo "To use the tools"
	@echo "----------------"
	@echo "module load $(DESTPREF)/tools-oss"
	@echo "	   - or -"
	@echo "source $(DESTPREF)/tools-oss.sh"
	@echo "	   - or -"
	@echo "source $(DESTPREF)/tools-oss.csh"
	@echo =======================================================================
	@echo

