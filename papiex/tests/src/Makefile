# Removed omp.c for now. We don't have a compiler for OpenMP.
SRCS = 	basic.c 
#badpthreads.c badpthreads2.c close.c fork.c pthreads.c assert.c abort.c 
#	sigint.c exit.c dlopen.c _exit.c write.c puts.c papiex_flops.c 
#	fork.c execl.c execlp.c system.c forkexecl.c forkexeclp.c shlogin.c cshlogin.c omp.c
#FSRCS = barrier.f90 
#SCRIPTS = runtests testexec testcsh testsh verify run_ex_tests bash_empty bash_fork_exec badps
#OBJS = $(SRCS:%.c=%.o) $(FSRCS:%.f90=%.o)
#NG_TESTS := basic-ng multiplex-ng memory-ng openmp-ng pthreads-ng csv-ng caliper-ng shell-ng syscalls-ng
#TESTS = $(OBJS:%.o=%) caliper2 caliper_c caliper_f fullcaliper_c fullcaliper_f gccinstr pthreadgccinstr \
	papiex_thr_flops papiex_thr_instr_flops

TEST_CASES := $(patsubst $(root-dir)/tests/src/%.sh,%,$(wildcard $(root-dir)/tests/src/[0-9]-*-ng.sh))
TARGETS := basic badps ng-driver $(TEST_CASES)
CFLAGS := -g -O1
PAPIEX_FLAGS += -I$(root-dir)/src
PAPIEX_LIBS += -L$(build-dir) -lpapiex $(LIBS)

.PHONY: all
all: $(TARGETS)
	@echo $(TARGETS)
#$(TARGETS)
#
%: %.sh
	cp -p $< $@

.PHONY: clean
clean: 
	-rm -f *~ core.* *.o *.a *.txt

.PHONY: clobber
clobber: clean
	-rm -f $(TARGETS)
