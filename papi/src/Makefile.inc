PAPI_SRCDIR = $(PWD)
SOURCES	  = $(MISCSRCS) papi.c papi_internal.c papi_hl.c extras.c sw_multiplex.c \
    papi_fwrappers.c papi_fwrappers_.c papi_fwrappers__.c upper_PAPI_FWRAPPERS.c \
    threads.c cpus.c $(OSFILESSRC) $(CPUCOMPONENT_C) papi_preset.c \
    papi_vector.c papi_memory.c $(COMPSRCS)
OBJECTS = $(MISCOBJS) papi.o papi_internal.o papi_hl.o extras.o sw_multiplex.o \
    papi_fwrappers.o papi_fwrappers_.o papi_fwrappers__.o upper_PAPI_FWRAPPERS.o \
    threads.o cpus.o $(OSFILESOBJ) $(CPUCOMPONENT_OBJ) papi_preset.o \
    papi_vector.o papi_memory.o $(COMPOBJS)
PAPI_EVENTS_TABLE = papi_events_table.h
HEADERS  = $(MISCHDRS) $(OSFILESHDR) $(PAPI_EVENTS_TABLE) \
	papi.h papi_internal.h papiStdEventDefs.h \
	papi_preset.h threads.h cpus.h papi_vector.h \
	papi_memory.h config.h \
	extras.h sw_multiplex.h papi_hl.h \
	papi_common_strings.h components_config.h

LIBCFLAGS += -I. $(CFLAGS) -DOSLOCK=\"$(OSLOCK)\" -DOSCONTEXT=\"$(OSCONTEXT)\"
FHEADERS = fpapi.h f77papi.h f90papi.h
# pkgconfig directory
LIBPC = $(LIBDIR)/pkgconfig

all: $(SHOW_CONF) $(LIBS) utils tests 
.PHONY : all test fulltest tests testlib utils ctests ftests comp_tests validation_tests null

include $(COMPONENT_RULES)

showconf:
		@echo "Host architecture   : $(DESCR)";
		@echo "Host CPU component  : $(CPUCOMPONENT_NAME)"; 
		@echo "Installation DESTDIR: $(DESTDIR)"; 
		@echo "Installation PREFIX : $(PREFIX)"; 
		@echo "Installation EPREFIX: $(EPREFIX)"; 
		@echo "Installation INCDIR : $(INCDIR)"; 
		@echo "Installation LIBDIR : $(LIBDIR)"; 
		@echo "Installation BINDIR : $(BINDIR)"; 
		@echo "Installation MANDIR : $(MANDIR)"; 
		@echo "Installation DOCDIR : $(DOCDIR)"; 
		@echo "Installation DATADIR: $(DATADIR)"; 
		@echo

show_bgp_conf:
	@echo;
	@echo "BG/P System Path   :  $(BGP_SYSDIR)";
	@echo "BG/P Install Path  :  $(BGP_INSTALLDIR)";
	@echo "BG/P GNU/Linux Path:  $(BGP_GNU_LINUX_PATH)";
	@echo "BG/P ARCH Path     :  $(BGP_ARCH_PATH)";
	@echo "BG/P Runtime Path  :  $(BGP_RUNTIME_PATH)";
	@echo

lib: $(SHOW_CONF) $(LIBS)

static: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	rm -f $(LIBRARY)
	$(AR) $(ARG64) rv $(LIBRARY) $(OBJECTS)

shared: libpapi.so libpapi.so.$(PAPIVER)

libpapi.so libpapi.so.$(PAPIVER): $(SHLIB)
	ln -sf $(SHLIB) $@

$(SHLIB): $(HEADERS) $(SOURCES) $(SHLIBOBJS)
	rm -f $(SHLIB) libpapi.so libpapi.so.$(PAPIVER); 
	$(CC_SHR) $(LIBCFLAGS) $(OPTFLAGS) $(SOURCES) $(SHLIBOBJS) -o $@ $(SHLIBDEPS) $(LDFLAGS)
	@set -ex; if test "$(POST_BUILD)" != "" ; then \
		-$(POST_BUILD) ; \
	fi

papi_fwrappers_.c: papi_fwrappers.c $(HEADERS)
	$(CPP) $(CPPFLAGS) -DFORTRANUNDERSCORE papi_fwrappers.c > papi_fwrappers_.c

papi_fwrappers__.c: papi_fwrappers.c $(HEADERS)
	$(CPP) $(CPPFLAGS) -DFORTRANDOUBLEUNDERSCORE papi_fwrappers.c > papi_fwrappers__.c

upper_PAPI_FWRAPPERS.c: papi_fwrappers.c $(HEADERS)
	$(CPP) $(CPPFLAGS) -DFORTRANCAPS papi_fwrappers.c > upper_PAPI_FWRAPPERS.c

papi_fwrappers.o: papi_fwrappers.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi_fwrappers.c -o papi_fwrappers.o

papi_fwrappers_.o: papi_fwrappers_.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi_fwrappers_.c -o papi_fwrappers_.o 

papi_fwrappers__.o: papi_fwrappers__.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi_fwrappers__.c -o papi_fwrappers__.o

upper_PAPI_FWRAPPERS.o: upper_PAPI_FWRAPPERS.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c upper_PAPI_FWRAPPERS.c -o upper_PAPI_FWRAPPERS.o

papi.o: papi.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi.c -o papi.o

papi_internal.o: papi_internal.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi_internal.c -o papi_internal.o

threads.o: threads.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c threads.c -o threads.o

cpus.o: cpus.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c cpus.c -o cpus.o

papi_hl.o: papi_hl.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi_hl.c -o papi_hl.o

aix-memory.o: aix-memory.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c aix-memory.c -o aix-memory.o

solaris-memory.o: solaris-memory.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c solaris-memory.c -o solaris-memory.o

solaris-common.o: solaris-common.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c solaris-common.c -o solaris-common.o

linux-bgp-memory.o: linux-bgp-memory.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c linux-bgp-memory.c -o linux-bgp-memory.o

linux-bgq-memory.o: linux-bgq-memory.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c linux-bgq-memory.c -o linux-bgq-memory.o

darwin-memory.o: darwin-memory.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c darwin-memory.c -o darwin-memory.o

darwin-common.o: darwin-common.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c darwin-common.c -o darwin-common.o

linux-memory.o: linux-memory.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c linux-memory.c -o linux-memory.o

linux-timer.o: linux-timer.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c linux-timer.c -o linux-timer.o

linux-common.o: linux-common.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c linux-common.c -o linux-common.o

extras.o: extras.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c extras.c -o extras.o

papi_memory.o: papi_memory.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi_memory.c -o papi_memory.o

papi_vector.o: papi_vector.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi_vector.c -o papi_vector.o

papi_preset.o: papi_preset.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c papi_preset.c -o papi_preset.o

sw_multiplex.o: sw_multiplex.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c sw_multiplex.c -o sw_multiplex.o

$(CPUCOMPONENT_OBJ): $(CPUCOMPONENT_C) $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c $(CPUCOMPONENT_C) -o $(CPUCOMPONENT_OBJ) 

x86_cpuid_info.o: x86_cpuid_info.c x86_cpuid_info.h $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c x86_cpuid_info.c -o x86_cpuid_info.o

$(PAPI_EVENTS_TABLE): $(PAPI_EVENTS_CSV) papi_events_table.sh
	sh papi_events_table.sh $(PAPI_EVENTS_CSV) > $@

$(ARCH_EVENTS)_map.o: $(ARCH_EVENTS)_map.c $(HEADERS)
	$(CC) $(LIBCFLAGS) $(OPTFLAGS) -c $(ARCH_EVENTS)_map.c -o $(ARCH_EVENTS)_map.o

# Required for BGP
.SUFFIXES: .rts.o
.c.rts.o:
	$(CC) $(CFLAGS) -c $< -o $@

bgp_tests:$(LIBRARY) null
	$(SETPATH) $(MAKE) -C ctests/bgp CC="$(CC)" CC_R="$(CC_R)" MPICC="$(MPICC)" CFLAGS="-I.. -I../.. $(CFLAGS)" TOPTFLAGS="$(TOPTFLAGS)" SMPCFLGS="$(SMPCFLGS)" OMPCFLGS="$(OMPCFLGS)" NOOPT="$(NOOPT)" LDFLAGS="$(LDFLAGS) $(STATIC)" LIBRARY="../../$(LINKLIB)" bgp_tests
 
#Required for freebsd
freebsd-memory.o: freebsd-memory.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map.o: freebsd/map.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-unknown.o: freebsd/map-unknown.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-p6.o: freebsd/map-p6.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-p6-m.o: freebsd/map-p6-m.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-p6-3.o: freebsd/map-p6-3.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-p6-2.o: freebsd/map-p6-2.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-p6-c.o: freebsd/map-p6-c.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-k7.o: freebsd/map-k7.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-k8.o: freebsd/map-k8.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-p4.o: freebsd/map-p4.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-atom.o: freebsd/map-atom.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-core.o: freebsd/map-core.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-core2.o: freebsd/map-core2.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-core2-extreme.o: freebsd/map-core2-extreme.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-i7.o: freebsd/map-i7.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

freebsd/map-westmere.o: freebsd/map-westmere.c $(HEADERS)
	$(CC) $(LIBCFLAGS) -c $< -o $@

test: ctests
	$(SETPATH) ctests/zero

fulltest: tests
	$(SETPATH) sh run_tests.sh

tests: $(TESTS)

testlib:
	$(SETPATH) $(MAKE) -C testlib

utils: $(LIBS) testlib
	$(MAKE) -C utils

validation_tests:	$(LIBS) testlib
	$(SETPATH) $(MAKE) -C validation_tests

ctests: $(LIBS) testlib validation_tests
	$(SETPATH) $(MAKE) -C ctests

ftests: $(LIBS) testlib
	$(SETPATH) $(MAKE) -C ftests




# compile tests added to components
comp_tests:	$(LIBS) testlib
ifneq (${COMPONENTS},)
	@set -ex; for comp in ${COMPONENTS} ; do \
		$(SETPATH) $(MAKE) -C components/$$comp/tests ; \
	done
endif

clean: comp_tests_clean native_clean
	-$(MAKE) -C ../doc clean
	-$(MAKE) -C ctests clean
	-$(MAKE) -C ftests clean
	-$(MAKE) -C testlib clean
	-$(MAKE) -C utils clean
	-$(MAKE) -C validation_tests clean
	rm -rf $(LIBRARY) $(SHLIB) libpapi.so libpapi.so.$(PAPIVER) $(OBJECTS) core rii_files genpapifdef *~ so_locations papi_fwrappers_.c papi_fwrappers__.c upper_PAPI_FWRAPPERS.c

# Component tests cleaning
comp_tests_clean:
ifneq (${COMPONENTS},)
	@set -ex; for comp in ${COMPONENTS} ; do \
		$(MAKE) -C components/$$comp/tests clean ; \
	done 
endif

clobber distclean: clean native_clobber 
	-$(MAKE) -C ../doc distclean
	-$(MAKE) -C ctests distclean
	-$(MAKE) -C ftests distclean
	-$(MAKE) -C testlib distclean
	-$(MAKE) -C utils distclean
	-$(MAKE) -C validation_tests distclean
	-$(MAKE) -C components -f Makefile_comp_tests distclean
	rm -f $(LIBRARY) $(SHLIB) $(EXTRALIBS) Makefile config.h libpapi.so papi.pc components_config.h $(PAPI_EVENTS_TABLE) config.log config.status f77papi.h f90papi.h fpapi.h

null:

dist: 
	$(MAKE) install-all PREFIX=`pwd`/papi-$(CPUCOMPONENT_NAME)
	tar cfv ./papi-$(CPUCOMPONENT_NAME).tar ./papi-$(CPUCOMPONENT_NAME)
	gzip ./papi-$(CPUCOMPONENT_NAME).tar
	rm -rf ./papi-$(CPUCOMPONENT_NAME)

install-all: install install-tests

install: install-lib install-headers install-data install-man install-utils install-pkgconf

install-headers: 
	@echo "Headers (INCDIR) being installed in: \"$(DESTDIR)$(INCDIR)\""; 
	install -d $(DESTDIR)$(INCDIR)
	cp $(FHEADERS) papi.h papiStdEventDefs.h $(DESTDIR)$(INCDIR)
	cd $(DESTDIR)$(INCDIR); chmod go+r $(FHEADERS) papi.h papiStdEventDefs.h

install-data:
	install -d $(DESTDIR)$(DATADIR)
	cp -f ./papi_events.csv $(DESTDIR)$(DATADIR)

install-lib: lib install-headers native_install
	@echo "Libraries (LIBDIR) being installed in: \"$(DESTDIR)$(LIBDIR)\""; 
	-mkdir -p $(DESTDIR)$(LIBDIR)
	-chmod go+rx $(DESTDIR)$(LIBDIR)
ifneq (,$(findstring static,$(LIBS)))
	cp $(LIBRARY) $(DESTDIR)$(LIBDIR) && \
	    chmod go+r $(DESTDIR)$(LIBDIR)/$(LIBRARY)
endif 
ifneq (,$(findstring shared,$(LIBS)))
	cp $(SHLIB) $(DESTDIR)$(LIBDIR)/libpapi.so.$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC) && \
	    chmod go+r $(DESTDIR)$(LIBDIR)/libpapi.so.$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC) && \
 	    cd $(DESTDIR)$(LIBDIR) && ln -sf libpapi.so.$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC) libpapi.so.$(PAPIVER).$(PAPIREV).$(PAPIAGE) && \
	    cd $(DESTDIR)$(LIBDIR) && ln -sf libpapi.so.$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC) libpapi.so.$(PAPIVER) && cd $(DESTDIR)$(LIBDIR); ln -sf libpapi.so.$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC) libpapi.so
endif

install-man:  
	$(MAKE) -C ../man DOCDIR=$(DESTDIR)$(DOCDIR) MANDIR=$(DESTDIR)$(MANDIR) install

install-utils: utils
	$(SETPATH) $(MAKE) -C utils BINDIR="$(DESTDIR)$(BINDIR)" CC="$(CC)" CC_R="$(CC_R)" CFLAGS="-I.. $(CFLAGS)" TOPTFLAGS="$(TOPTFLAGS)" SMPCFLGS="$(SMPCFLGS)" OMPCFLGS="$(OMPCFLGS)" NOOPT="$(NOOPT)" LDFLAGS="$(LDFLAGS) $(STATIC)" LIBRARY="../$(LINKLIB)" install

install-tests:  install-comp_tests
	$(SETPATH) $(MAKE) -C testlib install
	$(SETPATH) $(MAKE) -C ctests install
	$(SETPATH) $(MAKE) -C ftests install
	$(SETPATH) $(MAKE) -C validation_tests install
	-cp run_tests.sh $(DESTDIR)$(DATADIR)
	-cp run_tests_exclude_cuda.txt $(DESTDIR)$(DATADIR)
	-cp run_tests_exclude.txt $(DESTDIR)$(DATADIR)
	-chmod go+rx $(DESTDIR)$(DATADIR)/run_tests.sh
	-chmod go+r $(DESTDIR)$(DATADIR)/run_tests_exclude_cuda.txt $(DESTDIR)$(DATADIR)/run_tests_exclude.txt

# Component tests installing
install-comp_tests:
ifneq (${COMPONENTS},)
	@set -ex; for comp in ${COMPONENTS} ; do \
		$(MAKE) -C components/$$comp/tests DATADIR="$(DESTDIR)$(DATADIR)/components" install ; \
	done
endif

install-pkgconf:
	@echo "pkcongfig being installed in: \"$(DESTDIR)$(LIBPC)\"";
	-mkdir -p $(DESTDIR)$(LIBPC)
	-chmod go+rx $(DESTDIR)$(LIBPC)
	cp papi.pc $(DESTDIR)$(LIBPC)/papi-$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC).pc
	cd $(DESTDIR)$(LIBPC); ln -sf papi-$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC).pc papi-$(PAPIVER).$(PAPIREV).$(PAPIAGE).pc;
	cd $(DESTDIR)$(LIBPC); ln -sf papi-$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC).pc papi-$(PAPIVER).pc;
	cd $(DESTDIR)$(LIBPC); ln -sf papi-$(PAPIVER).$(PAPIREV).$(PAPIAGE).$(PAPIINC).pc papi.pc;

#
# Dummy targets for configurations that do not also include a Rules file with targets
#
native_clean:

native_install:

native_clobber:
