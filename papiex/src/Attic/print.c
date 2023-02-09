static void print_executable_info(FILE *output, int csv) {
  char tool_version[128];
  char tool_build[128];
  snprintf(tool_version, 128, "%s version", tool);
  snprintf(tool_build, 128, "%s build", tool);
  fprintf(output,"%-30s: %s\n", tool_version, PAPIEX_VERSION);
  fprintf(output,"%-30s: %s/%s\n", tool_build, build_date, build_time);
  fprintf(output,"%-30s: %s\n", "Executable", hwinfo->fullname);
  fprintf(output,"%-30s: %s\n", "Arguments", process_args);
  fprintf(output,"%-30s: %s\n", "Processor", hwinfo->model_string);
  fprintf(output,"%-30s: %f\n", "Max MHz", hwinfo->cpu_max_mhz);
  fprintf(output,"%-30s: %s\n", "Hostname", hostname);
  fprintf(output,"%-30s: %s\n", "Options", getenv(PAPIEX_ENV));
  fprintf(output,"%-30s: %d\n", "Parent id", (int)getppid());
  fprintf(output,"%-30s: %d\n", "Process id", (int)pid);
  fprintf(output,"%-30s: %d\n", "Session id", (int)getsid(pid));
  fprintf(output,"%-30s: %d\n", "MPI Rank", myrank);
  fprintf(output,"%-30s: %d\n", "Num threads", all_threads_num);
}

static void print_rusage_stats(FILE *output) {
  struct rusage ru;
  getrusage(RUSAGE_SELF,&ru);
  pretty_printl(output, "[PROCESS] getrusage resident max KB", 0, ru.ru_maxrss);
  // pretty_printl(output, "Shared", 0, ru.ru_ixrss);
  // pretty_printl(output, "Data", 0, ru.ru_idrss);
  // pretty_printl(output, "Stack", 0, ru.ru_isrss);
  pretty_printl(output, "[PROCESS] getrusage minor faults", 0, ru.ru_minflt);
  pretty_printl(output, "[PROCESS] getrusage major faults", 0, ru.ru_majflt);
  // pretty_printl(output, "Swaps", 0, ru.ru_nswap);
  pretty_printl(output, "[PROCESS] getrusage block inputs", 0, ru.ru_inblock);
  pretty_printl(output, "[PROCESS] getrusage block outputs", 0, ru.ru_oublock);
  //  pretty_printl(output, "Msgs sent", 0, ru.ru_msgsnd);
  // pretty_printl(output, "Msgs received", 0, ru.ru_msgrcv);
  // pretty_printl(output, "Signals", 0, ru.ru_nsignals);
  pretty_printl(output, "[PROCESS] getrusage voluntary preemptions", 0, ru.ru_nvcsw);
  pretty_printl(output, "[PROCESS] getrusage involuntary preemptions", 0, ru.ru_nivcsw);
}

static void print_proc_stats(FILE *output, int csv)
{
  unsigned long long proc_minflt = 0;
  unsigned long long proc_majflt = 0;
  unsigned long long proc_utime_jifs = 0;
  unsigned long long proc_stime_jifs = 0;
  unsigned long long proc_nthr = 0;
  unsigned long long proc_starttime_jifs = 0;
  unsigned long long proc_blkio_delays_jifs = 0;
  unsigned long long proc_guesttime_jifs = 0;

  /* Get timestamp of process start from /proc */
  get_proc_stat_fields(&proc_minflt, &proc_majflt, &proc_utime_jifs, &proc_stime_jifs, &proc_nthr, &proc_starttime_jifs, &proc_blkio_delays_jifs, &proc_guesttime_jifs);
  
  pretty_printl(output, "[PROCESS] proc minor faults", 0, proc_minflt);
  pretty_printl(output, "[PROCESS] proc major faults", 0, proc_majflt);
  pretty_printl(output, "[PROCESS] proc utime jiffies", 0, proc_utime_jifs);
  pretty_printl(output, "[PROCESS] proc stime jiffies", 0, proc_stime_jifs);
  pretty_printl(output, "[PROCESS] proc num threads", 0, proc_nthr);
  pretty_printl(output, "[PROCESS] proc start time after boot jiffies", 0, proc_starttime_jifs);
  pretty_printl(output, "[PROCESS] proc block IO delays jiffies", 0, proc_blkio_delays_jifs);
  pretty_printl(output, "[PROCESS] proc guest time jiffies", 0, proc_guesttime_jifs);
}

static void print_counters(FILE *output, papiex_perthread_data_t *thread) 
{
  fprintf(output,"%-16lld [PROCESS] Wallclock usecs\n", 
	  (long long int)((1000000 * proc_fini_time.tv_sec + proc_fini_time.tv_usec) -
			  (1000000 * proc_init_time.tv_sec + proc_init_time.tv_usec)));
  fprintf(output,"%-16lld Real usecs\n",thread->data[0].real_usec);
  fprintf(output,"%-16lld Real cycles\n",thread->data[0].real_cyc);
  fprintf(output,"%-16lld Virtual usecs\n",thread->data[0].virt_usec);
  fprintf(output,"%-16lld Virtual cycles\n",thread->data[0].virt_cyc);
  
  if (!no_mpi_prof && is_mpied) {
    fprintf(output,"%-16lld MPI cycles\n",thread->papiex_mpiprof);
    fprintf(output,"%-16lld MPI Sync cycles\n",thread->papiex_mpisyncprof);
  }
  
  if (!no_io_prof)
    fprintf(output,"%-16lld IO cycles\n",thread->papiex_ioprof);
  
  if (!no_threadsync_prof)
    fprintf(output,"%-16lld Thr Sync cycles\n",thread->papiex_threadsyncprof);
  
  for (i=0;i<eventcnt;i++)
    fprintf(output,"%-16lld %s\n",thread->data[0].counters[i],eventnames[i]);
  for (j=1;j<thread->max_caliper_entries;j++) {
    if (thread->data[j].used >0) {
      const char* label = thread->data[j].label ;
      /* if the user didn't set a label string use the calliper index */
      char index_s[32];
      if (label == NULL) {
	snprintf(index_s, sizeof(index_s), "Caliper %d", j);
	label = index_s;
      }
      fprintf(output,"\n%-16lld [LABEL] %s\n",(long long)j, label);
      fprintf(output,"%-16lld [%s] Measurements\n",(long long)thread->data[j].used, label);
      fprintf(output,"%-16lld [%s] Real cycles\n",thread->data[j].real_cyc, label);
#ifdef FULL_CALIPER_DATA
      fprintf(output,"%-16lld [%s] Real usecs\n",thread->data[j].real_usec, label);
      fprintf(output,"%-16lld [%s] Virtual usecs\n",thread->data[j].virt_usec, label);
      fprintf(output,"%-16lld [%s] Virtual cycles\n",thread->data[j].virt_cyc, label);
      
      if (!no_mpi_prof && is_mpied) {
	fprintf(output,"%-16lld [%s] MPI cycles\n",thread->data[j].mpiprof, label);
	fprintf(output,"%-16lld [%s] MPI Sync cycles\n",thread->data[j].mpisyncprof, label);
      }
      
      if (!no_io_prof)
	fprintf(output,"%-16lld [%s] IO cycles\n",thread->data[j].ioprof, label);
      
      if (!no_threadsync_prof)
	fprintf(output,"%-16lld [%s] Thr Sync cycles\n",thread->data[j].threadsyncprof, label);
#endif
      for (i=0;i<eventcnt;i++)
	fprintf(output,"%-16lld [%s] %s\n",thread->data[j].counters[i], label, eventnames[i]);
    }
  }
   
}

/* This function should not probably be used to print
 * rusage and memory info, as that may not be thread-specific
 */
static void print_thread_stats(FILE *output, papiex_perthread_data_t *thread,
                               unsigned long tid, int scope, float process_walltime_usec)
{
  if (output == NULL) 
    return;

  // Header 

  print_executable_info(output,csv_output);
  if (_papiex_threaded) 
    fprintf(output,"%-30s: %lu\n", "Thread id", tid);
  
  // Data

  if (rusage) 
    print_rusage_stats(output);
  if (memory) 
    _papiex_dump_memory_info(output);
  if (proc_stats)
    print_proc_stats(output, csv_output);
  print_counters(output, thread);
}



static void print_banner(FILE *stream) {
  assert(stream);
  assert(tool);
  assert(PAPIEX_VERSION);
  assert(build_date);
  assert(build_time);
  char email[1024] = "";
#ifdef MAILBUGS
  sprintf(email, "Send bug reports to %s", MAILBUGS);
#endif
  fprintf(stream, "%s:\n%s: %s (Build %s/%s)\n%s: %s\n",
                   tool, tool, PAPIEX_VERSION, build_date, 
                   build_time, tool, email);
  fflush(stream);
}
