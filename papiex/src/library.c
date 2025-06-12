#include "papiex_internal.h"

static void init_library_globals(void) 
{
  static char *this_tool = "papiex";

  tool = this_tool;

  // PAPI

  eventcnt = 0;
  memset(eventnames,0x0,sizeof(char *)*PAPIEX_MAX_COUNTERS);
  memset(eventcodes,0x0,sizeof(int)*PAPIEX_MAX_COUNTERS);
  // hwinfo = NULL;
#ifdef HAVE_PAPI
  exeinfo = NULL;
#endif
  
  // Process

  memset(output_prefix,0x0,sizeof(output_prefix));
  memset(process_hostname,0x0,sizeof(process_hostname));
  memset(process_args,0x0,sizeof(process_args));
  memset(process_name,0x0,sizeof(process_name));
  memset(process_fullname,0x0,sizeof(process_fullname));
  memset(process_tags,0x0,sizeof(process_tags));
  process_exitcode = 256; // Illegal
  process_exitsignal = 0; // 0 = normal

  // Monitoring data 

  all_threads_size = 0;
  all_threads_num = 0;
  all_threads = NULL;

  // Options

  _papiex_opt_quiet = 0;
  _papiex_opt_memory = 0;
  _papiex_opt_proc_stats = 0;
  _papiex_opt_multiplex = 0;
  _papiex_opt_rusage = 0;
  _papiex_opt_debug = 0;
  _papiex_opt_csv_v2 = 0;
  _papiex_mpi_numranks = 0;

  // state management

  called_process_shutdown = 0;

  // Linker handle 

  dlproc_handle = NULL;
}

int papiex_init_library(void) 
{
  char *opts = NULL, *tmp = NULL;
  static char maxopts[4096];
  int ecnt = 0;

  init_library_globals();

  dlproc_handle = monitor_real_dlopen(NULL, RTLD_LAZY);
  if (dlproc_handle == NULL) {
    LIBPAPIEX_ERROR("dlopen(NULL, RTLD_LAZY) failed. %s",strerror(errno));
    return -1;
  }

  _papiex_opt_debug = 0;
  tmp = getenv(PAPIEX_DEBUG_ENV);
  if (tmp) 
    _papiex_opt_debug = 1; 

  opts = getenv(PAPIEX_ENV);
  LIBPAPIEX_DEBUG("%s set to (%s)",PAPIEX_ENV,opts);
  if (opts == NULL) 
    return -1;

  // Iterate through options string and set options

  if (strlen(opts) > 0) {
    strncpy(maxopts,opts,sizeof(maxopts));
    tmp = strtok(maxopts,", ");
    if (tmp) 
      {
	do {
	  if (strcmp(tmp,"QUIET") == 0) 
	    _papiex_opt_quiet = 1;
	  else if (strcmp(tmp,"DEBUG") == 0)
	    _papiex_opt_debug++;
	  else if (strcmp(tmp,"MULTIPLEX") == 0)
	    _papiex_opt_multiplex = 1;
	  else if (strcmp(tmp,"RUSAGE") == 0)
	    _papiex_opt_rusage = 1;
	  else if (strcmp(tmp,"MEMORY") == 0)
	    _papiex_opt_memory = 1;
	  else if (strcmp(tmp,"PROC_STATS") == 0)
	    _papiex_opt_proc_stats = 1;
	  else if (strcmp(tmp,"COLLATED_TSV") == 0)
	    _papiex_opt_csv_v2 = 1;
	  else
	    {
	      if (ecnt >= PAPIEX_MAX_COUNTERS) {
		LIBPAPIEX_WARN("PAPIEX_MAX_COUNTERS %d reached, ignoring event %s",PAPIEX_MAX_COUNTERS,tmp);
		continue;
	      }
	      LIBPAPIEX_DEBUG("Event %d is %s",ecnt,tmp);
	      eventnames[ecnt] = strdup(tmp); // Global
	      if (eventnames[ecnt]) {
		ecnt++;
	      }
	    }
	} while ((tmp = strtok(NULL,",")) != NULL);
      }
  }

  eventcnt = ecnt;  // Global

  // If output is set, add hostname in front of it

  tmp = getenv(PAPIEX_OUTPUT_ENV);
  if ((tmp == NULL) || (strlen(tmp) == 0)) 
    strncpy(output_prefix,PAPIEX_DEFAULT_OUTPUT_PREFIX,sizeof(output_prefix));
  else 
    strncpy(output_prefix,tmp,sizeof(output_prefix));
  if (output_prefix[strlen(output_prefix)-1] != '/')
    strcat(output_prefix,"/");
  LIBPAPIEX_DEBUG("Output prefix is %s",output_prefix);
    
  return eventcnt;
}
