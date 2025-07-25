#include "papiex_internal.h"

void papiex_thread_init_routine(int tid, void *data)
{
  int real_tid = syscall(SYS_gettid);

  /* Should be able to defer this until later with static allocation? */

  LIBPAPIEX_DEBUG("THREAD(%d) INIT START malloc(%lu)",real_tid,(long unsigned int)sizeof(papiex_perthread_data_t));
  thr_data = malloc(sizeof(papiex_perthread_data_t));
  if (thr_data == NULL) {
    LIBPAPIEX_ERROR("malloc(%d) for per thread data failed.",(int)sizeof(papiex_perthread_data_t));
    return;
  }
  memset(thr_data,0x0,sizeof(papiex_perthread_data_t));
#ifdef HAVE_PAPI
  thr_data->papi_eventset = PAPI_NULL;
#endif
  thr_data->monitor_tid = tid;
  thr_data->real_tid = real_tid;
  thr_data->max_caliper_entries = PAPIEX_MAX_CALIPERS;

  /* Link in the thread struct to the process wide table for later retrieval (or if thread terminates) */

#ifdef HAVE_PAPI
  PAPI_lock(PAPI_LOCK_USR2);
#endif
  if (all_threads_num+1 >= all_threads_size) {
    LIBPAPIEX_DEBUG("Doubling size of thread table to %d entries",all_threads_size*2);
    all_threads = (papiex_perthread_data_t **)realloc(all_threads,all_threads_size*2*sizeof(papiex_perthread_data_t *));
    if (all_threads == NULL) {
      LIBPAPIEX_ERROR("malloc() for %d thread entries failed.",all_threads_size*2);
      free(thr_data);
#ifdef HAVE_PAPI
      PAPI_unlock(PAPI_LOCK_USR2);
#endif
      return;
    }
    all_threads_size = all_threads_size*2;
  }
  all_threads[all_threads_num] = thr_data;
  all_threads_num++;

#ifdef HAVE_PAPI
  PAPI_unlock(PAPI_LOCK_USR2);

  /* Now do the PAPI initialization */
  
  int retval, papi_eventset = PAPI_NULL;
  retval = PAPI_create_eventset(&papi_eventset);
  if (retval != PAPI_OK) {
    LIBPAPIEX_PAPI_ERROR("PAPI_create_eventset",retval);
    eventcnt = 0;
  } else {
    retval = PAPI_assign_eventset_component(papi_eventset, 0);
    if (retval != PAPI_OK) {
      LIBPAPIEX_PAPI_ERROR("PAPI_assign_eventset_component",retval);
      eventcnt = 0;
    } else {
      if (_papiex_opt_multiplex) {
	LIBPAPIEX_DEBUG("Calling PAPI_set_multiplex on the eventset");
	retval = PAPI_set_multiplex(papi_eventset);
	if (retval != PAPI_OK) {
	  LIBPAPIEX_PAPI_ERROR("PAPI_set_multiplex",retval);
	  eventcnt = 0;
	}
      }
    }
  }
 
  int i;

  /* Set up the events */

  int errcnt = 0;
  for (i=0; i<eventcnt; i++) {
    LIBPAPIEX_DEBUG("Calling PAPI_add_event(%d,0x%x) (%s)", papi_eventset,eventcodes[i],eventnames[i]);
    retval = PAPI_add_event(papi_eventset, eventcodes[i]);
    if (retval < PAPI_OK) {
      errcnt++;
      eventcodes[i] = 0;
      LIBPAPIEX_ERROR("PAPI_add_event(%s): %s",eventnames[i],PAPI_strerror(retval))
    }
  }
  
  /* The above can fail, for example if the user is missing CAP_SYS_ADMIN */

  if (errcnt - eventcnt == 0) {
    eventcnt = 0;
  } else if (errcnt) {
    /* If some fail, compact the eventcode and eventname array */
    for (i=0;i<eventcnt;i++) {
      if (eventcodes[i] == 0) {
	int j;
	for (j=i+1;j<eventcnt;j++) {
	  if (eventcodes[j] != 0) {
	    eventcodes[i] = eventcodes[j];
	    eventnames[i] = eventnames[j];
	    break;
	  }
	}
      }
    }
    eventcnt -= errcnt;
  }
  
  if (eventcnt) {
    thr_data->papi_eventset = papi_eventset;
    LIBPAPIEX_DEBUG("Starting counters with PAPI_start");
    retval = PAPI_start(thr_data->papi_eventset);
    if (retval != PAPI_OK) {
      LIBPAPIEX_PAPI_ERROR("PAPI_start",retval);
      eventcnt = 0;
    }
  } 
#endif
  
  // timestamp 

  struct timeval t1;
  gettimeofday(&t1,NULL);
  thr_data->start = t1.tv_sec*1000000 + t1.tv_usec;

  // go
  
#ifdef HAVE_PAPI
  papiex_start(0,"");
#endif
  LIBPAPIEX_DEBUG("THREAD(%d) INIT END",real_tid);
}

int extract_proc_task_stat(char *buf2, int len, unsigned long long *values)
{
  long num_threads = 0;
  unsigned long cminflt = 0, cmajflt = 0, cutime = 0, cstime = 0, guesttime = 0;
  unsigned long long starttime = 0, delaytime = 0;
  int processor = 0;
  int i=0;
  int field=1;
  char *buf = NULL;

  LIBPAPIEX_DEBUG("%s, %d, %p",buf2,len,values);
  if ((buf2 == NULL) || (len == 0) || (strlen(buf2) == 0)) {
    LIBPAPIEX_ERROR("extract_proc_task_stat(%p, %d, %p)",buf2,len,values);
    return 0;
  }
  if ((buf = strrchr(buf2,')')) == NULL) {
    LIBPAPIEX_ERROR("Could not find ) in task stat buffer %s",buf2);
    return 0;
  }
  buf++;
  if (*buf != ' ') {
    LIBPAPIEX_ERROR("Could not find space after ) in task stat buffer %s",buf2);
    return 0;
  }
  buf++;

  field = 2;
  len = strlen(buf);
  for (i=0;i<len;i++) {
    if (buf[i] == ' ') {
      field++;
      if (field == 10) 
	sscanf(buf+i+1,"%lu",&cminflt);
      else if (field == 12) 
	sscanf(buf+i+1,"%lu",&cmajflt);
      else if (field == 15) 
	sscanf(buf+i+1,"%lu",&cutime);
      else if (field == 16) 
	sscanf(buf+i+1,"%lu",&cstime);
      else if (field == 20) 
	sscanf(buf+i+1,"%ld",&num_threads);
      else if (field == 22) 
	sscanf(buf+i+1,"%llu",&starttime);
      else if (field == 39) 
	sscanf(buf+i+1,"%d",&processor);
      else if (field == 42) 
	sscanf(buf+i+1,"%llu",&delaytime);
      else if (field == 43) 
	sscanf(buf+i+1,"%lu",&guesttime);
    }
  }

  i = 9;
  unsigned long long clktck = 1000000/sysconf(_SC_CLK_TCK);
  values[0] = cminflt;
  values[1] = cmajflt;
  values[2] = clktck*(unsigned long long)cutime;
  values[3] = clktck*(unsigned long long)cstime;
  values[4] = num_threads;
  values[5] = clktck*(unsigned long long)starttime;
  values[6] = processor;
  values[7] = clktck*(unsigned long long)delaytime;
  values[8] = clktck*(unsigned long long)guesttime;

  LIBPAPIEX_DEBUG("%d values: cminflt %llu cmajflt %llu cutime %llu cstime %llu num_threads %llu starttime %llu processor %llu delaytime %llu guesttime %llu",
		  i, values[0],   values[1],  values[2],  values[3],       values[4],    values[5],        values[6],     values[7],    values[8])
  return i;
}

int get_proc_task_stat(int pid, int tid, papiex_plugin_data_t *p)
{
  static const char filename[] = "/proc/%d/task/%d/stat";
  static const char fields[] = "cminflt,cmajflt,cutime,cstime,num_threads,starttime,processor,delayacct_blkio_time,guest_time";
  //  static char fields[] = "pid,comm,state,ppid,pgrp,session,tty_nr,tpgid,flags,minflt,cminflt,majflt,cmajflt,utime,stime,cutime,cstime,priority,nice,num_threads,itrealvalue,starttime,vsize,rss,rsslim,startcode,endcode,startstack,kstkesp,kstkeip,signal,blocked,sigignore,sigcatch,wchan,nswap,cnswap,exit_signal,processor,rt_priority,policy,delayacct_blkio_ticks,guest_time,cguest_time,start_data,end_data,start_brk,arg_start,arg_end,env_start,env_end,exit_code";
  // static const char format[] = "%d (%s) %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %lld %lld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %d";
  int fd;
  char str[PATH_MAX];
  int len = sizeof(p->buf);

  memset(p,0x0,sizeof(papiex_plugin_data_t));

  sprintf(str,filename,pid,tid);
  if ((fd = open(str,O_RDONLY)) < 0) {
    LIBPAPIEX_ERROR("open(%s): %s",str,strerror(errno));
    return -1;
  }
  if ((p->len = read(fd,p->buf,len)) <= 0) {
    LIBPAPIEX_ERROR("read(%d->%s): %s",fd,str,strerror(errno));
    close(fd);
    return -1;
  }
  close(fd);
  // We can flush the newline
  p->buf[p->len-1] = '\0';
  strlcpy(p->fields,fields,sizeof(p->fields));

  // p->filename = filename;
  // p->format = format;
  //  p->version = 1;
  LIBPAPIEX_DEBUG("%s, length %d contents (%s)",filename,p->len,p->buf);

  return(p->len);
}

int get_proc_exeinfo()
{
  // sets globals process_name, process_fullname, process_args

  const char filename[] = "/proc/%d/cmdline";
  const char filename2[] = "/proc/%d/exe";
  // const char fields[] = "exename,path,args";
  int fd, len;
  char str[PATH_MAX] = "";
  char buf[PAPIEX_MAX_ARG_STRLEN] = ""; 
  int pid = getpid();
  
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

  for (j = retval+1; j <= len; j++) {
    if (buf[j] == '\0')
      process_args[i++] = ' ';
    else
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

int extract_getrusage(char *buf, int len, unsigned long long *values)
{
  struct rusage *ru = (struct rusage *)buf;
  if ((buf == NULL) || (len == 0)) {
    LIBPAPIEX_ERROR("extract_getrusage(%p, %d, %p)",buf,len,values);
    return 0;
  }
  if (len != sizeof(struct rusage)) {
    LIBPAPIEX_ERROR("size error %d vs %lu",len,sizeof(struct rusage));
    return 0;
  }

  int num = 9;
  values[0] = ru->ru_utime.tv_sec * 1000000 + ru->ru_utime.tv_usec;
  values[1] = ru->ru_stime.tv_sec * 1000000 + ru->ru_stime.tv_usec;
  values[2] = ru->ru_maxrss;
  values[3] = ru->ru_minflt;
  values[4] = ru->ru_majflt;
  values[5] = ru->ru_inblock;
  values[6] = ru->ru_oublock;
  values[7] = ru->ru_nvcsw;
  values[8] = ru->ru_nivcsw;
  LIBPAPIEX_DEBUG("%d values: utime %llu stime %llu maxrss %llu minflt %llu maxflt %llu inblock %llu outblock %llu nvcsw %llu nivcsw %llu",
		  num,values[0],values[1],values[2],values[3],values[4],values[5],values[6],values[7],values[8]);
  return num;
}

static int get_rusage(int pid, int tid, papiex_plugin_data_t *d) 
{
  // static const char filename[] = "getrusage(RUSAGE_THREAD)";
  static const char fields[] = "usertime,systemtime,rssmax,minflt,majflt,inblock,outblock,vol_ctxsw,invol_ctxsw";
  // static char format[] = "";
  struct rusage *ru = (struct rusage *)d->buf;
  
  memset(d,0x0,sizeof(papiex_plugin_data_t));

  if (sizeof(struct rusage) > sizeof(d->buf)) {
    LIBPAPIEX_ERROR("size of rusage %d greater than buffer size %d",
		    (int)sizeof(struct rusage),(int)sizeof(d->buf));
  }
  if (getrusage(RUSAGE_THREAD,ru) == -1) {
    LIBPAPIEX_ERROR("getrusage(): %s",strerror(errno));
    return -1;
  }
  LIBPAPIEX_DEBUG("getrusage user time %ld us", ru->ru_utime.tv_sec * 1000000 + ru->ru_utime.tv_usec);
  LIBPAPIEX_DEBUG("getrusage system time %ld us", ru->ru_stime.tv_sec * 1000000 + ru->ru_stime.tv_usec);
  LIBPAPIEX_DEBUG("getrusage resident max KB %ld", ru->ru_maxrss);
  LIBPAPIEX_DEBUG("getrusage minor faults %ld", ru->ru_minflt);
  LIBPAPIEX_DEBUG("getrusage major faults %ld", ru->ru_majflt);
  LIBPAPIEX_DEBUG("getrusage block inputs %ld", ru->ru_inblock);
  LIBPAPIEX_DEBUG("getrusage block outputs %ld", ru->ru_oublock);
  LIBPAPIEX_DEBUG("getrusage voluntary preemptions %ld", ru->ru_nvcsw);
  LIBPAPIEX_DEBUG("getrusage involuntary preemptions %ld", ru->ru_nivcsw);

  strlcpy(d->fields,fields,sizeof(d->fields));
  d->len = sizeof(struct rusage);
  //d->format = format;
  //d->version = 1;
  //d->filename = filename;
  return d->len;
}

int extract_proc_task_io(char *buf, int len, unsigned long long *values)
{
  int num = 7;
  /* rchar: 141115117
    wchar: 4685808
    syscr: 14142
    syscw: 4679
    read_bytes: 66326528
    write_bytes: 1073152
    cancelled_write_bytes: 0 */

  if ((buf == NULL) || (len == 0) || (strlen(buf) == 0)) {
    LIBPAPIEX_ERROR("extract_proc_task_io(%p, %d, %p)",buf,len,values);
    return 0;
  }

  if (sscanf(buf,"rchar: %llu\n",&values[0]) != 1) 
    { LIBPAPIEX_ERROR("scanf(%s) expected 1 value",buf); return 0; }
  buf=strchr(buf,'\n')+1;
  if (sscanf(buf,"wchar: %llu\n",&values[1]) != 1)
    { LIBPAPIEX_ERROR("scanf(%s) expected 1 value",buf); return 0; }
  buf=strchr(buf,'\n')+1;
  if (sscanf(buf,"syscr: %llu\n",&values[2]) != 1)
    { LIBPAPIEX_ERROR("scanf(%s) expected 1 value",buf); return 0; }
  buf=strchr(buf,'\n')+1;
  if (sscanf(buf,"syscw: %llu\n",&values[3]) != 1)
    { LIBPAPIEX_ERROR("scanf(%s) expected 1 value",buf); return 0; }
  buf=strchr(buf,'\n')+1;
  if (sscanf(buf,"read_bytes: %llu\n",&values[4]) != 1)
    { LIBPAPIEX_ERROR("scanf(%s) expected 1 value",buf); return 0; }
  buf=strchr(buf,'\n')+1;
  if (sscanf(buf,"write_bytes: %llu\n",&values[5]) != 1)
    { LIBPAPIEX_ERROR("scanf(%s) expected 1 value",buf); return 0; }
  buf=strchr(buf,'\n')+1;
  if (sscanf(buf,"cancelled_write_bytes: %llu\n",&values[6]) != 1)
    { LIBPAPIEX_ERROR("scanf(%s) expected 1 value",buf); return 0; }

  LIBPAPIEX_DEBUG("%d values: rchar %llu wchar %llu syscr %llu syscw %llu read_bytes %llu write_bytes %llu cancelled_write_bytes %llu",
		  num,values[0],values[1],values[2],values[3],values[4],values[5],values[6]);
  return num;
}


static int get_proc_task_io(int pid, int tid, papiex_plugin_data_t *d) 
{
  // Also used as name
  const char filename[] = "/proc/%d/task/%d/io";
  const char fields[] = "rchar,wchar,syscr,syscw,read_bytes,write_bytes,cancelled_write_bytes";
  // static char format[] = "";
  int fd;
  char str[PATH_MAX];
  int len = sizeof(d->buf);

  memset(d,0x0,sizeof(papiex_plugin_data_t));

  sprintf(str,filename,pid,tid);
  if ((fd = open(str,O_RDONLY)) < 0) {
    LIBPAPIEX_ERROR("open(%s): %s",str,strerror(errno));
    return -1;
  }
  if ((d->len = read(fd,d->buf,len)) <= 0) {
    LIBPAPIEX_ERROR("read(%d->%s): %s",fd,str,strerror(errno));
    close(fd);
    return -1;
  }
  close(fd);
  // We can flush the newline
  d->buf[d->len-1] = '\0';
  strlcpy(d->fields,fields,sizeof(d->fields));
  // d->format = format;
  // d->version = 1;
  // d->filename = filename;
  LIBPAPIEX_DEBUG("%s, length %d contents (%s)",filename,d->len,d->buf);

  return(d->len);
}

int extract_proc_task_schedstat(char *buf, int len, unsigned long long *values)
{
  int num = 3;
  if ((buf == NULL) || (len == 0) || (strlen(buf) == 0)) {
    LIBPAPIEX_ERROR("extract_proc_task_schedstat(%p, %d, %p)",buf,len,values);
    return 0;
  }
  if (sscanf(buf,"%llu %llu %llu",&values[0],&values[1],&values[2]) != 3) {
    LIBPAPIEX_ERROR("scanf(%s) expected 3 values",buf);
    return 0;
  }
  LIBPAPIEX_DEBUG("%d values: oncpu %llu waiting %llu slices %llu",
		  num,values[0],values[1],values[2]);
  return num;
}

static int get_proc_task_schedstat(int pid, int tid, papiex_plugin_data_t *d) 
{
  // Also used as name
  static const char filename[] = "/proc/%d/task/%d/schedstat";
  static const char fields[] = "time_oncpu,time_waiting,timeslices";
  // static char format[] = "%lu %lu %lu";
  int fd;
  char str[PATH_MAX];
  int len = sizeof(d->buf);

  memset(d,0x0,sizeof(papiex_plugin_data_t));

  sprintf(str,filename,pid,tid);
  if ((fd = open(str,O_RDONLY)) < 0) {
    LIBPAPIEX_ERROR("open(%s): %s",str,strerror(errno));
    return -1;
  }
  if ((d->len = read(fd,d->buf,len)) <= 0) {
    LIBPAPIEX_ERROR("read(%d->%s): %s",fd,str,strerror(errno));
    close(fd);
    return -1;
  }
  close(fd);
  // We can flush the newline
  d->buf[d->len-1] = '\0';
  strlcpy(d->fields,fields,sizeof(d->fields));
  // d->format = format;
  // d->version = 1;
  // d->filename = filename;
  LIBPAPIEX_DEBUG("%s, length %d contents (%s)",filename,d->len,d->buf);

  return(d->len);
}
    
void papiex_thread_shutdown_routine(void *data)
{
  LIBPAPIEX_DEBUG("THREAD(%d) SHUTDOWN START",thr_data->real_tid);

  // stop 

#ifdef HAVE_PAPI
  papiex_stop(0);
#endif
  
  // timestamp
  
  struct timeval t1;
  gettimeofday(&t1,NULL);
  thr_data->end = t1.tv_sec*1000000 + t1.tv_usec;

  // stuff from getrusage

  get_rusage(getpid(), thr_data->real_tid, &thr_data->plugin[0]);

  // stuff from proc/pid/task/tid/stat

  get_proc_task_stat(getpid(), thr_data->real_tid, &thr_data->plugin[1]);

  // stuff from proc/pid/task/tid/io

  get_proc_task_io(getpid(), thr_data->real_tid, &thr_data->plugin[2]);

  // stuff from proc/pid/task/tid

  get_proc_task_schedstat(getpid(), thr_data->real_tid, &thr_data->plugin[3]);

  // memory consistency flag that shows everything is up to date
  thr_data->papi_eventset = 0xdeadbeef;

  // Flush the per-thread data structure out of cache
  _papiex_cache_flush_thrdata(thr_data);
  
  // These use DEBUG for error messages as this may fail due to target clobbering PAPI state, like closing it's FD 
  // See similar code in papiex_stop

#ifdef HAVE_PAPI
  // save this for later PAPI cleanup, because we use this field to
  // indicate flush to memory.
  int retval, papi_eventset = thr_data->papi_eventset;

  if (eventcnt) {
    retval = PAPI_stop(papi_eventset,NULL);
    if (retval != PAPI_OK)
      LIBPAPIEX_DEBUG("PAPI_stop(pid %d,tid %d,exe %s) failed: %s",(int)getpid(),(int)syscall(SYS_gettid),process_name,PAPI_strerror(retval));

    retval = PAPI_cleanup_eventset(papi_eventset);
    if (retval != PAPI_OK)
      LIBPAPIEX_DEBUG("PAPI_cleanup_eventset(pid %d,tid %d,exe %s) failed: %s",(int)getpid(),(int)syscall(SYS_gettid),process_name,PAPI_strerror(retval));

    retval = PAPI_destroy_eventset(&papi_eventset);
    if (retval != PAPI_OK)
      LIBPAPIEX_DEBUG("PAPI_destroy_eventset(pid %d,tid %d,exe %s) failed: %s",(int)getpid(),(int)syscall(SYS_gettid),process_name,PAPI_strerror(retval));
  }
  
  // explicitly remove thread from papi

  retval = PAPI_unregister_thread();
  if (retval != PAPI_OK) 
    LIBPAPIEX_PAPI_ERROR("PAPI_unregister_thread",retval);
#endif
  
  LIBPAPIEX_DEBUG("THREAD(%d) SHUTDOWN END",thr_data->real_tid);
}

