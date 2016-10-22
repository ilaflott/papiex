PapiEx - PAPI Execute
=====================

PapiEx provides a command-line interface to PAPI for _shared executables_.


Dependencies
------------
* PAPI (> 4.0) : http://icl.cs.utk.edu/papi
* Libmonitor  : http://libmonitor.googlecode.com


Build PapiEx
------------
  1. Build and install PAPI library

  2. Build and install libmonitor

  3. Now build and install papiex:

        $ make PREFIX=/path/to/where/you/want/to/install/papiex \
               MONITOR_PREFIX=/path/monitor/install \
               PAPI_INC_PATH=/path/to/papi/include PAPI_LIB_PATH=/path/to/papi/lib/directory

        $ make install PREFIX=/path/to/where/you/want/to/install/papiex

        $ make quicktest (optional)
        $ make test (optional)


    Build time arguments that are honored:

    * `MONITOR_PREFIX`
    * `PAPI_PREFIX`, `PAPI_INC_PATH`, `PAPI_LIB_PATH`
    * `USE_MPI=1` Include MPI tests and allows MPI gather with --classic
	
    Note, PapiEx works for MPI programs even if `USE_MPI` is unset. However,
    adding USE_MPI allows papiex to be used with the --classic mode with
    MPI gathering of statistics.


Troubleshooting
---------------

If the PAPI libraries are not installed in the standard run time 
linker search path then you had better set the environment variable
`LD_LIBRARY_PATH` to point to the correct places. 

    $ setenv LD_LIBRARY_PATH /usr/local/lib:/opt/local/lib (for csh)
    $ export LD_LIBRARY_PATH /usr/local/lib:/opt/local/lib (for sh/bash/ksh)

MPI gather with --classic can be a source of bugs. Try not using --classic
or use it with --no-mpi-gather.
