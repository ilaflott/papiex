papiex - PAPI Execute
=====================

Building with PAPI
------------------
If you want to build with PAPI, please uncomment the -DHAVE_PAPI in the definition of the DEFINES variable in the Makefile. Please ensure you maintain the -DHAVE_MONITOR in that line as well.

How to Build/Test - Native
--------------------------

This distribution honors the PREFIX and DESTDIR variables standardized by GNU/FSF software. 

$ make PREFIX=/path/to/install/dir install

To verify things are working:

$ make PREFIX=/path/to/install/dir check

The output of the above test will move the data into ./sample-data.build

How to Build/Test - Docker
--------------------------

The default target is OS_TARGET=centos-7. If you want something else, then specify it on the make line. 

$ make docker-dist
$ make docker-test-dist

See Makefile and Dockerfiles directory for other platforms, note only Centos-7 is maintained, so you'll need to port the other Dockerfiles. 

Output of make check
--------------------

# make PREFIX=/usr/papi check  
cd papiex; make PREFIX=/usr/papi LIBPAPIEX=/usr/papi/lib/libpapiex.so check
make[1]: Entering directory '/Users/phil/Work/papiex-oss/papiex'
make -C /Users/phil/Work/papiex-oss/papiex/x86_64-Linux -f /Users/phil/Work/papiex-oss/papiex/src/Makefile check
make[2]: Entering directory '/Users/phil/Work/papiex-oss/papiex/x86_64-Linux'
cp -Rp /Users/phil/Work/papiex-oss/papiex/src/tests/* /Users/phil/Work/papiex-oss/papiex/x86_64-Linux/tests
cd tests; ./test.sh
/usr/papi/bin/monitor-run -i /usr/papi/lib/libpapiex.so
Testing papi with PERF_COUNT_SW_CPU_CLOCK...
/usr/papi/bin/papi_command_line PERF_COUNT_SW_CPU_CLOCK: PASS(0)
0 errors.

Testing tagged runs...
sleep 1: PASS(0)
ps -fade: PASS(0)
host google.com: PASS(0)
echo : | tr ':' '\n': PASS(0)
tcsh -f module-test.csh: PASS(0)
bash --noprofile -c 'sleep 1': PASS(0)
tcsh -f -c 'sleep 1': PASS(0)
csh -f -c 'sleep 1': PASS(0)
tcsh -f evilcsh.csh: PASS(0)
csh -f evilcsh.csh: PASS(0)
bash --noprofile sieve.sh 100: PASS(0)
tcsh -f sieve.csh 100: PASS(0)
csh -f sieve.csh 100: PASS(0)
python sieve.py: PASS(0)
perl sieve.pl: PASS(0)
gcc -Wall unit1.c -o unit1a: PASS(0)
gcc -pthread dotprod_mutex.c -o dotprod_mutex: PASS(0)
g++ -fopenmp md_openmp.cpp -o md_openmp: PASS(0)
gfortran -fopenmp fft_openmp.f90 -o fft_openmp: PASS(0)
./unit1a: PASS(0)
./dotprod_mutex: PASS(0)
./md_openmp: PASS(0)
./fft_openmp: PASS(0)
0 errors.
make[2]: Leaving directory '/Users/phil/Work/papiex-oss/papiex/x86_64-Linux'
make[1]: Leaving directory '/Users/phil/Work/papiex-oss/papiex'
ln -s /usr/papi/tmp ./sample-data.build