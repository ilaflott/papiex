#ifndef PAPIEX_INTERNAL_H
#define PAPIEX_INTERNAL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <fenv.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <memory.h>
#include <search.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "monitor.h"
#ifdef HAVE_PAPI
#include "papi.h"
#endif
#include "papiex.h"

#define MPIP_ENV "MPIP"
#define PAPIEX_DEFAULT_ARGS "PAPIEX_DEFAULT_ARGS"
#define PAPIEX_ENV "PAPIEX_OPTIONS"
#define PAPIEX_DEBUG_ENV "PAPIEX_DEBUG"
#define PAPIEX_OUTPUT_ENV "PAPIEX_OUTPUT"
#define PAPIEX_TAGS_ENV "PAPIEX_TAGS"
#define PAPIEX_LDLP "LD_LIBRARY_PATH"
#define PAPIEX_MAX_ARG_STRLEN 32*4096
#define PAPIEX_MAX_CSV_STRLEN 2*PAPIEX_MAX_ARG_STRLEN
#define PAPIEX_MAX_COUNTERS 16
#define PAPIEX_INIT_THREADS 128
#define PAPIEX_MAX_LABEL_SIZE 64
#define PAPIEX_MAX_CALIPERS 1
#define PAPIEX_MAX_PLUGINS 4

#define PAPIEX_DEBUG_THREADID (unsigned long)(monitor_get_thread_num())
#define LIBPAPIEX_DEBUG(...) { if (_papiex_opt_debug) { fprintf(stderr,"libpapiex: debug %lu,0x%lx,%s ",(unsigned long)getpid(),PAPIEX_DEBUG_THREADID,__func__); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n"); } }
// #define PAPIEX_DEBUG(...) { extern char *tool; if (debug) { fprintf(stderr,"%s: debug %s",tool,__func__); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n"); } }
// #define PAPIEX_PAPI_DEBUG(name,code) { PAPIEX_DEBUG("PAPI debug from %s, %d, %s", name, code, PAPI_strerror(code)); }

#define LIBPAPIEX_WARN(...) {  fprintf(stderr,"libpapiex: warning "); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n"); }
#define LIBPAPIEX_ERROR(...) { fprintf(stderr,"libpapiex: error "); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n"); }
#ifdef HAVE_PAPI
#define LIBPAPIEX_PAPI_ERROR(name,code) { LIBPAPIEX_ERROR("from %s, %d, %s", name, code, PAPI_strerror(code)); }
#define LIBPAPIEX_PAPI_WARN(name,code) {  LIBPAPIEX_WARN("from %s, %d, %s", name, code, PAPI_strerror(code)); }
#endif

			     // #define PAPIEX_WARN(...) { extern char *tool;  fprintf(stderr,"%s: warning ",tool); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n"); }
			     // #define PAPIEX_ERROR(...) { extern char *tool; fprintf(stderr,"%s: error ",tool); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n"); exit(1); }
			     // #define PAPIEX_PAPI_ERROR(name,code) { PAPIEX_ERROR("PAPI error from %s, %d, %s", name, code, PAPI_strerror(code)); }
			     // #define PAPIEX_PAPI_WARN(name,code) { PAPIEX_WARN("PAPI warning from %s, %d, %s", name, code, PAPI_strerror(code)); }

#define IO_PROFILE_LIB "libpapiexio.so"
#define THREADSYNC_PROFILE_LIB "libpapiexmpsync.so"
#define MPI_PROFILE_LIB "libpapiexmpi.so"

extern void _papiex_dump_event_info(FILE *out, int eventcode, int verbose);
extern void _papiex_dump_memory_info(FILE *out);

typedef struct {
  char label[PAPIEX_MAX_LABEL_SIZE];
  float used;
  int depth;
  long long tmp_real_cyc;
  long long real_cyc;
  long long tmp_counters[PAPIEX_MAX_COUNTERS];
  long long counters[PAPIEX_MAX_COUNTERS];
} papiex_caliper_data_t;

typedef struct {
  char buf[4096];
  int len;
  char fields[1020];
  // char *filename;
  // const char *format;
  // int version;
} papiex_plugin_data_t; 

typedef struct {
  int monitor_tid;
  int real_tid;
  int papi_eventset;
  int mpi_rank;
  long long start;    /* GTOD thread start time */
  long long end;      /* GTOD thread stop time */
  int max_caliper_entries; /* number of entries in allocated structure */
  int max_caliper_used; /* highest number of referenced entry */
  papiex_caliper_data_t data[PAPIEX_MAX_CALIPERS];
  papiex_plugin_data_t plugin[PAPIEX_MAX_PLUGINS];
} papiex_perthread_data_t;

#define STRINGIFY(X) #X
#define GET_DLSYM(dl_handle,name,real_ptr,fptr_type) \
    if (real_ptr == NULL) { \
      char* err; \
      dlerror(); \
      real_ptr = (fptr_type)dlsym(dl_handle, name);	\
      if ((err = dlerror()) != NULL) { \
        LIBPAPIEX_ERROR("dlsym(dl_handle,%s) failed. %s", name, err);	\
        real_ptr = NULL; \
      } \
      if (real_ptr)  LIBPAPIEX_DEBUG("%s : %p\n", name, real_ptr); \
    }

#endif

/* Prototypes */

void papiex_thread_init_routine(int tid, void *data);
void papiex_thread_shutdown_routine(void *data);
int papiex_process_init_routine(int argc, char **argv, void *data);
void papiex_process_shutdown_routine(int, void *data);
int papiex_init_library(void); 
int papiex_write_data_csv(char *output_pref, papiex_perthread_data_t *all[], int num, char *en[], int ecnt);
int papiex_write_data_csv_v2(char *output_pref, papiex_perthread_data_t *all[], int num, char *en[], int ecnt);

/* Evil globals */

extern __thread papiex_perthread_data_t *thr_data;
extern int eventcnt;
extern int _papiex_opt_debug;
extern int _papiex_opt_csv_v2;
extern int _papiex_multiplex;
extern char *eventnames[PAPIEX_MAX_COUNTERS];
extern int eventcodes[PAPIEX_MAX_COUNTERS];
extern int all_threads_num;
extern int all_threads_size;
extern papiex_perthread_data_t **all_threads;
extern char output_prefix[PATH_MAX];
extern char process_args[PAPIEX_MAX_ARG_STRLEN];
//extern const PAPI_hw_info_t *hwinfo;
#ifdef HAVE_PAPI
extern const PAPI_exe_info_t *exeinfo;
#endif
extern char process_name[PATH_MAX];
extern char process_fullname[PATH_MAX];
extern char process_tags[PATH_MAX];
extern int process_exitcode;
extern int process_exitsignal;
extern char process_hostname[PATH_MAX];
extern int _papiex_mpi_numranks;
extern int called_process_shutdown;
extern void *dlproc_handle;
extern int _papiex_opt_quiet;
extern int _papiex_opt_memory;
extern int _papiex_opt_proc_stats;
extern int _papiex_opt_multiplex;
extern int _papiex_opt_rusage;
extern char *tool;
extern const char *build_date;
extern const char *build_time;

#if (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#include <stdint.h>
// From Intel's libpmem
#define FLUSH_ALIGN ((uintptr_t)64)
#define force_inline __attribute__((always_inline)) inline
static force_inline void cache_flush_range(const void *addr, size_t len)
{
        uintptr_t uptr;

        /*                                                                                                                                     
         * Loop through cache-line-size (typically 64B) aligned chunks                                                                         
         * covering the given range.                                                                                                           
         */
        for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
                uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN)
                _mm_clflush((char *)uptr);
}

static force_inline void _papiex_cache_flush_thrdata(const papiex_perthread_data_t *thr_data)
{
  _mm_mfence();
  cache_flush_range(&thr_data->data[0],sizeof(papiex_caliper_data_t));
  cache_flush_range(&thr_data->plugin[0],sizeof(papiex_plugin_data_t));
  cache_flush_range(thr_data,sizeof(papiex_perthread_data_t));
  _mm_mfence();
}

#else
#error "_papiex_cache_flush not implemented for this architecture!"
#endif

// Defined in output.c
#ifndef HAVE_STRLCAT
extern size_t strlcpy(char *dst, const char *src, size_t size);
#endif
#ifndef HAVE_STRLCPY
extern size_t strlcat(char *dst, const char *src, size_t size);
#endif
