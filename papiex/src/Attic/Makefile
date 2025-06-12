
#-L. -lpapiex -Wl,-rpath -Wl,$(build-dir)
# $(build-dir)/libpapiex.so.1.3.0 -L$(LIBDIR) -lmonitor -lpapi 
#-L$(LIBDIR) -lmonitor -lpapi 
#-lmonitor 
#
#papiex: $(DRIVER_OBJS)
#	$(CC) $(CFLAGS) $(DEFINES) $* -o $@ -L$(LIBDIR) -lpapi -Wl,-rpath -Wl,$# (LIBDIR)

#
# Junk
# 

uninstall:
	-cd $(DESTDIR)$(LIBDIR); rm -f libpapiexio.so libpapiexio.so.$(INTERFACE) libpapiexio.so.$(INTERFACE).$(REVISION).$(AGE)
	-cd $(DESTDIR)$(LIBDIR); rm -f libpapiexmpsync.so libpapiexmpsync.so.$(INTERFACE) libpapiexmpsync.so.$(INTERFACE).$(REVISION).$(AGE)
ifneq (,$(findstring libpapiexmpi.so,$(TARGETS)))
	-cd $(DESTDIR)$(LIBDIR); rm -f libpapiexmpi.so libpapiexmpi.so.$(INTERFACE) libpapiexmpi.so.$(INTERFACE).$(REVISION).$(AGE)
endif
	-cd $(DESTDIR)$(BINDIR); rm -f papiex
	-cd $(DESTDIR)$(SHAREDIR); rm -f papiex.sh papiex.csh

libpapiexpathscale.so: pathscale.c 
	$(CC) $(CFLAGS) $(DEFINES) -DUSE_PAPIEX $(SHRCFLAGS) $(SHRLDFLAGS) $^ -Wl,-soname -Wl,$@.$(INTERFACE) $(LDFLAGS) -ldl -o $@.$(INTERFACE).$(REVISION).$(AGE) && \
	ln -sf $@.$(INTERFACE).$(REVISION).$(AGE) $@.$(INTERFACE) && \
	ln -sf $@.$(INTERFACE) $@

libpapiexgcc.so: gcc.c
	$(CC) $(CFLAGS) $(DEFINES) -DUSE_PAPIEX $(SHRCFLAGS) $(SHRLDFLAGS) $^ -Wl,-soname -Wl,$@.$(INTERFACE) $(LDFLAGS) -ldl -o $@.$(INTERFACE).$(REVISION).$(AGE) && \
	ln -sf $@.$(INTERFACE).$(REVISION).$(AGE) $@.$(INTERFACE) && \
	ln -sf $@.$(INTERFACE) $@

libpapiexio.so: iowrap.c
	$(CC) $(CFLAGS) $(DEFINES) $(SHRCFLAGS) $(SHRLDFLAGS) $^ -Wl,-soname -Wl,$@.$(INTERFACE) $(LDFLAGS) -ldl -o $@.$(INTERFACE).$(REVISION).$(AGE) && \
	ln -sf $@.$(INTERFACE).$(REVISION).$(AGE) libpapiexio.so.$(INTERFACE) && \
	ln -sf libpapiexio.so.$(INTERFACE) libpapiexio.so

libpapiexmpsync.so: threadsyncwrap.c
	$(CC) $(CFLAGS) $(DEFINES) $(SHRCFLAGS) $(SHRLDFLAGS) $^ -Wl,-soname -Wl,$@.$(INTERFACE) $(LDFLAGS) -ldl -o $@.$(INTERFACE).$(REVISION).$(AGE) && \
	ln -sf $@.$(INTERFACE).$(REVISION).$(AGE) libpapiexmpsync.so.$(INTERFACE) && \
	ln -sf libpapiexmpsync.so.$(INTERFACE) libpapiexmpsync.so

libpapiexmpi.so: mpiwrap.c
	$(MPICC) $(CFLAGS) $(DEFINES) $(SHRCFLAGS) $(SHRLDFLAGS) $^ -Wl,-soname -Wl,$@.$(INTERFACE) $(LDFLAGS) -ldl -o $@.$(INTERFACE).$(REVISION).$(AGE) && \
	ln -sf $@.$(INTERFACE).$(REVISION).$(AGE) libpapiexmpi.so.$(INTERFACE) && \
	ln -sf libpapiexmpi.so.$(INTERFACE) libpapiexmpi.so

papiex-report: papiex-report.pl
	cp $< $@

papiex-output2csv: papiex-output2csv.py
	cp $< $@

threadsyncwrap.c: threadsyncwrap.in makewraplib.pl
	perl $(root-dir)/src/makewraplib.pl < $(root-dir)/src/threadsyncwrap.in > $@

iowrap.c: iowrap.in makewraplib.pl
	perl $(root-dir)/src/makewraplib.pl < $(root-dir)/src/iowrap.in > $@

mpiwrap.c: mpiwrap_special.c mpiwrap.in makempiwraplib.pl
	perl $(root-dir)/src/makempiwraplib.pl -f < $(root-dir)/src/mpiwrap.in > $@
	cat $(root-dir)/src/mpiwrap_special.c >> $@

libiberty.so: $(LIBIBERTY_STATIC)
	$(LD) -rpath $(LIBDIR) -whole-archive -soname $@ $< -G -o $@

libbfd.so: $(LIBBFD_STATIC)
	$(LD) -rpath $(LIBDIR) -whole-archive -soname $@ $< -G -o $@

papiex-specs: specs
	-rm -frv $@
	cp -r $< $@

ifneq (,$(findstring PROFILING_SUPPORT,$(DEFINES)))
  TARGETS += libpapiexio.so libpapiexmpsync.so
  ifneq (,$(findstring USE_MPI,$(DEFINES)))
    TARGETS += libpapiexmpi.so 
  endif
endif

papiex-config: papiex-config.in
	@sed -e 's\PREFIX\$(PREFIX)\g; s\EXECPREFIX\$(EXECPREFIX)\g; s\INCDIR\$(INCDIR)\g; s\LIBDIR\$(LIBDIR)\g; s\MANDIR\$(MANDIR)\g' < $< > $@
	@chmod +x papiex-config

