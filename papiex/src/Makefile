SO_OBJS=api.o thread.o process.o library.o papiex.o monitor.o output.o

.PHONY: all lib
all lib: $(SONAME) $(SOVERSION) $(SOBASE) 

$(SO_OBJS): %.o: %.c papiex_internal.h papiex.h
	$(CC_PTHR) $(CFLAGS) $(SHRCFLAGS) $(DEFINES) -I$(root-dir)/src $(INCLUDES) -c $<

$(SONAME): $(SO_OBJS)
	$(CC_PTHR) $(SHRLDFLAGS) $(SO_OBJS) -Wl,-soname -Wl,$(SOVERSION) -o $(SONAME) -L$(LIBDIR) -Wl,-rpath -Wl,$(LIBDIR) $(LIBS)

$(SOVERSION): $(SONAME)
	ln -sf $(SONAME) $(SOVERSION)

$(SOBASE): $(SOVERSION)
	ln -sf $(SOVERSION) $(SOBASE) 

.PHONY: test check
test check: 
	cp -Rp $(root-dir)/src/tests/* $(build-dir)/tests
	cd $(build-dir)/tests; PAPIEX_PREFIX=$(PREFIX) ./test.sh

.PHONY: install
install: all
	install $(build-dir)/$(SONAME) $(DESTDIR)$(LIBDIR) 
	cd $(DESTDIR)$(LIBDIR); ln -fs $(SONAME) $(SOVERSION); ln -fs $(SOVERSION) $(SOBASE)

.PHONY: uninstall
uninstall:
	-cd $(DESTDIR)$(LIBDIR); rm -f $(SONAME) $(SOVERSION) $(SOBASE)
