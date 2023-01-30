#include "papiex_internal.h"
static volatile int _papiex_enable = -1;
#define CHECK_FOR_DISABLE_AND_FORK(retval) if (_papiex_enable == -1) return retval; if (_papiex_enable != getpid()) { LIBPAPIEX_ERROR("%s: _papiex_enable %d vs getpid() %d",__func__,_papiex_enable,getpid()); return retval; }
void monitor_init_library(void) 
{
  LIBPAPIEX_DEBUG("monitor_init_library() callback");
  if (papiex_init_library() != -1) {
    _papiex_enable = getpid();
  }
  LIBPAPIEX_DEBUG("monitor_init_library() callback, _papiex_enable = %d",_papiex_enable);
}

void* monitor_init_process(int *argc, char **argv, void* data)
{
  LIBPAPIEX_DEBUG("monitor_init_process(%d,%p,%p) callback",*argc,argv,data);
  if (_papiex_enable == -1)
    return data;
  if (_papiex_enable != getpid()) {
    LIBPAPIEX_DEBUG("monitor_init_process(): reinitializing papiex old pid was %d",_papiex_enable);
    _papiex_enable = -1;
    if (papiex_init_library() == -1) {
      LIBPAPIEX_DEBUG("monitor_init_process(): papiex library initialization failed, _papiex_enable = -1");
      return data;
    }
  }
  if (papiex_process_init_routine(*argc, argv, data)) {
    LIBPAPIEX_DEBUG("monitor_init_process(): papiex_process_init_routine failed, _papiex_enable = %d",_papiex_enable);
    return data;
  }
  _papiex_enable = getpid();
  LIBPAPIEX_DEBUG("monitor_init_process(): success, _papiex_enable = %d",_papiex_enable);
  return(data);
}

void monitor_fini_process(int how, void* data)
{
  LIBPAPIEX_DEBUG("monitor_fini_process(%d,%p) callback",how,data);
  CHECK_FOR_DISABLE_AND_FORK();
  papiex_process_shutdown_routine(how, data);
  _papiex_enable = -1;
}

void monitor_begin_process_exit(int how, int code)
{
  LIBPAPIEX_DEBUG("monitor_begin_process_exit(%d,%d) callback",code,how);
  process_exitcode = code;
  process_exitsignal = (how == MONITOR_EXIT_SIGNAL);
}

void monitor_init_thread_support(void)
{
  LIBPAPIEX_DEBUG("monitor_init_thread_support() callback");
  CHECK_FOR_DISABLE_AND_FORK();
  
#ifdef HAVE_PAPI
  int retval = PAPI_thread_init((unsigned long (*)(void))monitor_get_thread_num);
  if (retval != PAPI_OK) {
    LIBPAPIEX_DEBUG("Error %s from PAPI_thread_init",PAPI_strerror(retval));
    return;
  }
#endif
  LIBPAPIEX_DEBUG("monitor_init_thread_support() is threaded!");
}

void *monitor_init_thread(int tid, void *data)
{
  LIBPAPIEX_DEBUG("monitor_init_thread(%d,%p) callback",tid,data);
  CHECK_FOR_DISABLE_AND_FORK(data);
  papiex_thread_init_routine(tid,data);
  return data;
}

void monitor_fini_thread(void *data)
{
  LIBPAPIEX_DEBUG("monitor_fini_thread(%p) callback",data);
  CHECK_FOR_DISABLE_AND_FORK();
  papiex_thread_shutdown_routine(data);
}

void monitor_init_mpi(int *argc, char ***argv) 
{
  LIBPAPIEX_DEBUG("monitor_init_mpi(%d,%p) callback",*argc,argv);
  CHECK_FOR_DISABLE_AND_FORK();
  _papiex_mpi_numranks = monitor_mpi_comm_size();
  thr_data->mpi_rank = monitor_mpi_comm_rank();
  LIBPAPIEX_DEBUG("mpi_rank=%d, mpi_size=%d", thr_data->mpi_rank, _papiex_mpi_numranks);
}

void monitor_fini_mpi(void) 
{
  LIBPAPIEX_DEBUG("monitor_fini_mpi() callback");
  CHECK_FOR_DISABLE_AND_FORK();
}
