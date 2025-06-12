#include "papiex_internal.h"

char *tool = "papiex";
const char *build_date = __DATE__;
const char *build_time = __TIME__;
papiex_perthread_data_t **all_threads;
int all_threads_size; 
int all_threads_num; 

char output_prefix[PATH_MAX];
char process_hostname[PATH_MAX];
char process_args[PAPIEX_MAX_ARG_STRLEN];
char process_name[PATH_MAX];
char process_fullname[PATH_MAX];
char process_tags[PATH_MAX];
int process_exitcode;
int process_exitsignal;

// const PAPI_hw_info_t *hwinfo;
#ifdef HAVE_PAPI
const PAPI_exe_info_t *exeinfo;
#endif

int called_process_shutdown;
__thread papiex_perthread_data_t *thr_data = NULL;

int eventcnt = 0;
char *eventnames[PAPIEX_MAX_COUNTERS];
int eventcodes[PAPIEX_MAX_COUNTERS];

int _papiex_opt_multiplex;
int _papiex_opt_rusage;
int _papiex_opt_memory;
int _papiex_opt_proc_stats;
int _papiex_opt_quiet;
int _papiex_opt_debug;
int _papiex_opt_csv_v2;
void *dlproc_handle;
int _papiex_mpi_numranks;
/* int mpi_myrank is defined in per-thread structure */
