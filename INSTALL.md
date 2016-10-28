PapiEx - PAPI Execute
=====================

PapiEx provides a command-line interface to PAPI for *shared executables*.


Dependencies
------------
* PAPI (> 4.0) : http://icl.cs.utk.edu/papi
* Libmonitor   : http://libmonitor.googlecode.com

Both these dependencies are included in this papiex git repository. 
You also have the option build papiex against a pre-installed version of
PAPI or libmonitor.

The only requirement is the availability of gcc and standard *nix utilities
such as `make` and `autoconf`. Your system needs to support shared libraries.
Papiex has been well tested on many versions of Linux/x86_64. 


Getting the sources
-------------------
You can get papiex and it dependencies (PAPI and libmonitor) at:

    $ git clone https://bitbucket.org/minimalmetrics/papiex-oss.git


Build PapiEx
------------

The easiest way to build and use papiex is to use the bundled
PAPI and libmonitor. The `PREFIX` argument is optional.

      $ make install PREFIX=/path/to/where/you/want/to/install/papiex 

To use an exisiting PAPI installation, and not use the bundled PAPI:

      $ make PAPI_INC_PATH=/path/to/papi/install/include \
             PAPI_LIB_PATH=/path/to/papi/install/lib

      $ make test (optional)

      $ make fulltest (optional)

Similarly you can use `MONITOR_INC_PATH` and `MONITOR_LIB_PATH` to 
use an existing libmonitor. Please note papiex uses a version of 
monitor that comes out of the SciDAC project.

Build time arguments that are honored:

 * `MONITOR_PREFIX`, `MONITOR_INC_PATH`, and `MONITOR_LIB_PATH`
 * `PAPI_PREFIX`, `PAPI_INC_PATH`, and `PAPI_LIB_PATH`
 * `PROFILING_SUPPORT=1` to count I/O, MPI and thread-synchronization cycles
 * `USE_MPI=1` to build MPI tests
	
Note, PapiEx works for MPI programs even if `USE_MPI` is unset. However,
adding USE_MPI builds tests for MPI, and if `PROFILING_SUPPORT=1` is set
then time spent in MPI is also reported. 

To use papiex, see [README.md](README.md)

Troubleshooting
---------------

If the PAPI libraries are not installed in the standard run time 
linker search path then you had better set the environment variable
`LD_LIBRARY_PATH` to point to the correct places. 

    $ setenv LD_LIBRARY_PATH /usr/local/lib:/opt/local/lib (for csh)
    $ export LD_LIBRARY_PATH /usr/local/lib:/opt/local/lib (for sh/bash/ksh)
