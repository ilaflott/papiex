#include "papiex_internal.h"

#ifndef HAVE_PAPI
static int get_proc_exeinfo()
{
  // sets globals process_name, process_fullname, process_args

  const char filename[] = "/proc/%d/cmdline";
  const char filename2[] = "/proc/%d/exe";
  // const char fields[] = "exename,path,args";
  int fd, len;
  char str[PATH_MAX] = "";
  char buf[PAPIEX_MAX_ARG_STRLEN] = ""; 
  int pid = getpid();
  
  process_fullname[0] = '\0';
  process_args[0] = '\0';
  process_name[0] = '\0';

    sprintf(str,filename,pid);
  if ((fd = open(str,O_RDONLY)) < 0) {
    LIBPAPIEX_ERROR("open(%s): %s",str,strerror(errno));
    return -1;
  }
  if ((len = read(fd,buf,sizeof(buf))) <= 0) {
    LIBPAPIEX_ERROR("read(%d->%s): %s",fd,str,strerror(errno));
    close(fd);
    return -1;
  }
  close(fd);
  LIBPAPIEX_DEBUG("%s: length %d contents (%s)",str,len,buf);

  int j = 0, i = 0;
  int retval;
  retval = strlcpy(process_name,buf,sizeof(process_name)); // First null term entry is argv[0]

  for (j = retval+1; j < len; j++) {
    if (buf[j] == '\0') {
      if (j == len-1) // Last one we keep as NULL byte
	process_args[i++] = '\0';
      else // Separator replaced by space
	process_args[i++] = ' ';
    } else // Copy characters
      process_args[i++] = buf[j];
  }

  sprintf(str,filename2,pid);
  if ((len = readlink(str,buf,sizeof(buf))) <= 0) {
    LIBPAPIEX_ERROR("read(%d->%s): %s",fd,str,strerror(errno));
    return -1;
  }
  buf[len] = '\0'; // Append NULL to string, readlink doesn't do NULL term
  LIBPAPIEX_DEBUG("%s: length %d contents (%s)",str,len,buf);
  strlcpy(process_fullname,buf,sizeof(process_fullname));

  LIBPAPIEX_DEBUG("PROCESS_NAME:(%s)",process_name);
  LIBPAPIEX_DEBUG("PROCESS_FULLPATH:(%s)",process_fullname);
  LIBPAPIEX_DEBUG("PROCESS_ARGS:(%s)",process_args);
  
  return(0);
}
#endif

int papiex_process_init_routine(int argc, char **argv, void *data)
{
  int i;

  called_process_shutdown = 0;
  LIBPAPIEX_DEBUG("PROCESS INIT START %d,%p -> %s,%p",argc,argv,(argv ? argv[0] : ""),data);

  gethostname(process_hostname,sizeof(process_hostname));
  process_args[0] = '\0';
  for (i=1;i<argc;i++) {
    if (argv && argv[i]) {
      LIBPAPIEX_DEBUG("Process argv[%d]: %p",i,argv[i]);
      if (i > 1)
	strlcat(process_args," ",PAPIEX_MAX_ARG_STRLEN - strlen(process_args) - 1);
      strlcat(process_args,argv[i],PAPIEX_MAX_ARG_STRLEN - strlen(process_args) - 1);
    }
  }
  LIBPAPIEX_DEBUG("Process args: %s",process_args);
  char *env_tags = getenv(PAPIEX_TAGS_ENV);
  if (env_tags && (strlen(env_tags) != 0))
    strlcpy(process_tags, env_tags, sizeof(process_tags));
  LIBPAPIEX_DEBUG("Process tags: %s",process_tags);
  

#ifdef HAVE_PAPI
#if 0
  /* Crank up no of FD's */
  struct rlimit rl;
  if (getrlimit(RLIMIT_NOFILE,&rl) < 0)
    LIBPAPIEX_WARN("getrlimit(RLIMIT_NOFILE): %s",strerror(errno));
  rl.rlim_cur = rl.rlim_max;
  if (setrlimit(RLIMIT_NOFILE,&rl) < 0)
    LIBPAPIEX_WARN("setrlimit(RLIMIT_NOFILE): returned %s",strerror(errno));
  LIBPAPIEX_DEBUG("RLIMIT_NOFILE cur %d, max %d",(int)rl.rlim_cur,(int)rl.rlim_max);
#endif

  int retval;
  if (_papiex_opt_debug)
    {
      retval = PAPI_set_debug(PAPI_VERB_ECONT);
      if (retval != PAPI_OK)
	  LIBPAPIEX_PAPI_ERROR("PAPI_set_debug",retval);
      putenv("PAPI_DEBUG=API");
    }

  LIBPAPIEX_DEBUG("Initializing the PAPI library with version %#x",PAPI_VER_CURRENT);
  int version = PAPI_library_init(PAPI_VER_CURRENT);
  if (version < 0)
    {
      LIBPAPIEX_PAPI_ERROR("PAPI_library_init",version);
      return -1;
    }
  LIBPAPIEX_DEBUG("PAPI library returned with version %#x",version)
  if ((exeinfo = PAPI_get_executable_info()) == NULL)
    {
      LIBPAPIEX_PAPI_ERROR("PAPI_get_executable_info returned NULL",0);
      return -1;
    }
  LIBPAPIEX_DEBUG("Host is %s",process_hostname);
  strlcpy(process_name,(char *)exeinfo->address_info.name,sizeof(process_name));
  LIBPAPIEX_DEBUG("Process name: %s",process_name);
  strlcpy(process_fullname,(char *)exeinfo->fullname,sizeof(process_fullname));
  LIBPAPIEX_DEBUG("Process fullname: %s",process_fullname);

  int good = 0;
  for (i=0;i<eventcnt;i++)
    {
      int eventcode = 0;
      LIBPAPIEX_DEBUG("Checking event %s for existence.",eventnames[i]);
      retval = PAPI_event_name_to_code(eventnames[i],&eventcode);
      if (retval != PAPI_OK)
	{
          LIBPAPIEX_WARN("Could not map name to code for %s", eventnames[i]);
	  LIBPAPIEX_PAPI_ERROR("PAPI_event_name_to_code",retval);
	  eventcodes[i] = PAPI_NULL;
	}
      else
	{
	  eventcodes[i] = eventcode;
	  good++;
	}
    }
  eventcnt = good; // Still might be zero
#else
  get_proc_exeinfo();
#endif
  
  /* Initialize thread descriptor table */

  all_threads = (papiex_perthread_data_t **)malloc(PAPIEX_INIT_THREADS*sizeof(papiex_perthread_data_t *));
  if (all_threads == NULL) 
    {
      LIBPAPIEX_ERROR("malloc(%lu) for %d thread entries failed",PAPIEX_INIT_THREADS*sizeof(void *),all_threads_size);
      eventcnt = 0;
      return -1;
    }
  all_threads_size = PAPIEX_INIT_THREADS;
  all_threads_num = 0;
  
  papiex_thread_init_routine(monitor_get_thread_num(), NULL);

  LIBPAPIEX_DEBUG("PROCESS INIT END");
  return 0;
}

void papiex_process_shutdown_routine(int how, void *data)
{
  LIBPAPIEX_DEBUG("PROCESS SHUTDOWN START");
  
  // Skip duplicate shutdowns 

  if (called_process_shutdown) {
    LIBPAPIEX_WARN("PROCESS SHUTDOWN ALREADY CALLED (pid %d)", called_process_shutdown);
    return;
  }
  called_process_shutdown = getpid();

  // Skip those shutdowns that happen just before an exec. We will catch these at init. */
  
  if (how == MONITOR_EXIT_EXEC) {
   LIBPAPIEX_DEBUG("Skipping shutdown for a to-be-exec()'ed process");
   return;
  }

  // Shut down main thread

  LIBPAPIEX_DEBUG("Invoking thread shutdown in main thread");
  papiex_thread_shutdown_routine(all_threads[0]);

  // Ensure memory consistency, it happens

  int i;
  for (i=0;i<all_threads_num;i++) {
    papiex_perthread_data_t *t = all_threads[i];
    //    if (t->papi_eventset != 0xdeadbeef) {
    //      LIBPAPIEX_WARN("Stale data, thread %d, flushing cache...",i);
    _papiex_cache_flush_thrdata(t);
    //    }
  }
  
  papiex_write_data_csv_v2(output_prefix, all_threads, all_threads_num, eventnames, eventcnt);
  LIBPAPIEX_DEBUG("PROCESS SHUTDOWN END (num threads %d)", all_threads_num);
}
