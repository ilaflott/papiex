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
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <search.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

FILE **proc_stat_fp_to_close = NULL;
static void get_proc_stat_fields_after_main(void) __attribute__((destructor));
static void get_proc_stat_fields_after_main(void)
{
  if (proc_stat_fp_to_close) {
    FILE *t = *proc_stat_fp_to_close;
    *proc_stat_fp_to_close = NULL;
    fclose(t);
  }
}

static int get_proc_stat_fields(unsigned long long *minflt, unsigned long long *majflt, unsigned long long *utime_jifs, unsigned long long *stime_jifs, unsigned long long *nthr, unsigned long long *starttime_jifs, unsigned long long *iodelay_jifs, unsigned long long *guesttime_jifs)
{
  static FILE *fp = NULL;
  int rv; 
  if (fp == NULL) {
    char fn[256];
    FILE *t;

    sprintf(fn,"/proc/%lu/stat",(long unsigned int)getpid());
    t = fopen(fn, "r");
    if (t == NULL) // Quick bail
      return(-1);
    if (setvbuf(t, NULL, _IONBF, 0) < 0) { // Unbuffered
      fclose(t);
      return(-1);
    }
    fp = t;
    proc_stat_fp_to_close = &fp; // Close at _fini
  }
  rv = fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %llu %*u %llu %*u %llu %llu %*d %*d %*d %*d %llu %*d %llu %*u %*d %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d %*u %*u %llu %llu %*d", minflt, majflt, utime_jifs, stime_jifs, nthr, starttime_jifs, iodelay_jifs, guesttime_jifs);
  rewind(fp);
  return(rv);
}

//#define PAPIEX_UNIT_TEST
#ifdef PAPIEX_UNIT_TEST
int main()
{
  unsigned long long a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;;
  for (int i = 0; i < 100000; i++) {
    int rv = get_proc_stat_fields(&a, &b, &c, &d, &e, &f, &g, &h);
    if (i % 10000 == 0) {
      printf("rv %d minflt %llu majflt %llu utime %llu stime %llu nthr %llu starttime %llu iodelay %llu guesttime %llu\n",rv,a,b,c,d,e,f,g,h); 
      char fn[256];
      sprintf(fn,"cat /proc/%lu/stat",(long unsigned int)getpid());
      printf("%s:\n",fn);
      system(fn);
    }
  }
}
#endif

