SO_OBJS=api.o thread.o process.o library.o papiex.o monitor.o output.o

.PHONY: all lib
all lib: $(SONAME)

$(SO_OBJS): %.o: %.c papiex_internal.h papiex.h
	$(CC_PTHR) $(CFLAGS) $(SHRCFLAGS) $(DEFINES) -I$(root-dir)/src $(INCLUDES) -c $<

$(SONAME): $(SO_OBJS)
	$(CC_PTHR) $(SHRLDFLAGS) $(SO_OBJS) -Wl,-soname -Wl,$(SOVERSION) -o $(SONAME) -L$(LIBDIR) -Wl,-rpath -Wl,$(LIBDIR) $(LIBS)

.PHONY: test check
test check: $(SONAME)
	cp -Rp $(root-dir)/src/tests/* $(build-dir)/tests
	cd $(build-dir)/tests; PAPIEX_PREFIX=$(PREFIX) ./test.sh

