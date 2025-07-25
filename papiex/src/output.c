#include "papiex_internal.h"

#include <string.h>
#include <limits.h>     /* PATH_MAX */
#include <sys/stat.h>   /* mkdir(2) */
#include <errno.h>
#include <libgen.h>

#ifndef HAVE_STRLCAT
/*
 * '_cups_strlcat()' - Safely concatenate two strings.
 */

size_t                  /* O - Length of string */
strlcat(char       *dst,        /* O - Destination string */
              const char *src,      /* I - Source string */
          size_t     size)      /* I - Size of destination string buffer */
{
  size_t    srclen;         /* Length of source string */
  size_t    dstlen;         /* Length of destination string */


 /*
  * Figure out how much room is left...
  */

  dstlen = strlen(dst);
  size   -= dstlen + 1;

  if (!size)
    return (dstlen);        /* No room, return immediately... */

 /*
  * Figure out how much room is needed...
  */

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size)
    srclen = size;

  memcpy(dst + dstlen, src, srclen);
  dst[dstlen + srclen] = '\0';

  return (dstlen + srclen);
}
#endif /* !HAVE_STRLCAT */

#ifndef HAVE_STRLCPY
/*
 * '_cups_strlcpy()' - Safely copy two strings.
 */

size_t                  /* O - Length of string */
strlcpy(char       *dst,        /* O - Destination string */
              const char *src,      /* I - Source string */
          size_t      size)     /* I - Size of destination string buffer */
{
  size_t    srclen;         /* Length of source string */


 /*
  * Figure out how much room is needed...
  */

  size --;

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size)
    srclen = size;

  memcpy(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}
#endif /* !HAVE_STRLCPY */

int mkdir_p(const char *path)
{
  /* Adapted from http://stackoverflow.com/a/2336245/119527 */
  const size_t len = strlen(path);
  char _path[PATH_MAX];
  char *p; 

  LIBPAPIEX_DEBUG("%s",path);
  errno = 0;

  /* Copy string so its mutable */
  if (len > sizeof(_path)-1) {
    errno = ENAMETOOLONG;
    LIBPAPIEX_ERROR("mkdir_p(%p): %s",path,strerror(errno));
    return -1; 
  }   
  strcpy(_path, path);

  /* Iterate the string */
  for (p = _path + 1; *p; p++) {
    if (*p == '/') {
      /* Temporarily truncate */
      *p = '\0';
      LIBPAPIEX_DEBUG("mkdir(%s)",_path);
      if (mkdir(_path, S_IRWXU) != 0) {
	if (errno != EEXIST) {
	  LIBPAPIEX_ERROR("mkdir(%s): %s",_path,strerror(errno));
	  return -1;
	} else {
	  LIBPAPIEX_DEBUG("mkdir(%s): %s",_path,strerror(errno));
	}
      }
      *p = '/';
    }
  }   

  if (mkdir(_path, S_IRWXU) != 0) {
    if (errno != EEXIST) {
      LIBPAPIEX_ERROR("mkdir(%s): %s",_path,strerror(errno));
      return -1; 
    } else {
      LIBPAPIEX_DEBUG("mkdir(%s): %s",_path,strerror(errno));
    }
  }

  return 0;
}

static int create_process_file(char *prefix, char *fn, int *gen)
{
  int fd, instance = 0;
  char rfn[PATH_MAX];

  LIBPAPIEX_DEBUG("%s,%s,%p -> %d",prefix,fn,gen,*gen);
  if (mkdir_p(prefix) == -1)
    return -1;
  
  while (1) {
    snprintf(rfn,PATH_MAX,"%s%s-%s-%d-%d.csv",prefix,process_hostname,"papiex",getpid(),instance);
    LIBPAPIEX_DEBUG("trying to open file %s",rfn);
    fd = open(rfn, O_WRONLY|O_CREAT|O_EXCL, 0644);
    if (fd >= 0)
      break;
    else if (errno != EEXIST) {
      LIBPAPIEX_ERROR("open(%s): %s",rfn,strerror(errno));
      return -1;
    }
    LIBPAPIEX_DEBUG("%s exists",rfn);
    instance++;
  }
  *gen = instance;
  strncpy(fn,rfn,PATH_MAX);
  LIBPAPIEX_DEBUG("%s is file descriptor %d",rfn,fd);
  return fd;
}

static int write_buffer(char *fn, int fd, char *line)
{
  int l = strlen(line) * sizeof(char);
  char *nl = "\n";
  LIBPAPIEX_DEBUG("writing %d bytes to %s, fd %d",l,fn,fd);
  int rl = write(fd,line,l);
  // Append CR
  if (rl == l) {
    int rlnl = write(fd,nl,strlen(nl)*sizeof(char));
    if (rlnl == strlen(nl)*sizeof(char))
      return 0;
    rl = rlnl;
    line = nl;
  }
  if (rl < 0) {
    LIBPAPIEX_ERROR("write(%d -> %s,%p -> %s,%d) returned error %s",fd,fn,line,line,l,strerror(errno));
    return -1;
  }
  LIBPAPIEX_ERROR("write(%d -> %s,%p -> %s,%d) returned short %d",fd,fn,line,line,l,rl);
  return -1;
}

static int write_buffer_v2(const char *fn, int fd, char *buf, int buf_len)
{
  int len = strlcat(buf,"\n",buf_len);   // Append newline and count space
  if (len == buf_len) // meaning we maxed out
    return ENOSPC;
  LIBPAPIEX_DEBUG("pwrite(%d,%p,%ld,%d) -> %s",fd,buf,len*sizeof(char),0,fn);
  int rl = pwrite(fd,buf,len*sizeof(char),0);;
 if ((rl < 0) || (rl != len*sizeof(char))) {
   LIBPAPIEX_ERROR("pwrite(%d,%p,%ld,%d) -> %s returned %d, error %s",fd,buf,len*sizeof(char),0,fn,rl,strerror(errno));
   return -1;
 }
 LIBPAPIEX_DEBUG("pwrite(%d,%p,%ld,%d) -> %s returned %d",fd,buf,len*sizeof(char),0,fn,rl);
 return 0;
}

static int append_plugin_data(char *fn, char *line, int len, papiex_plugin_data_t *p, int plen, int no_leading_comma)
{
  extern int extract_getrusage(char *buf, int len, unsigned long long *values);
  extern int extract_proc_task_stat(char *buf, int len, unsigned long long *values);
  extern int extract_proc_task_io(char *buf, int len, unsigned long long *values);
  extern int extract_proc_task_schedstat(char *buf, int len, unsigned long long *values);
  int i;
  int retval,retval1,retval2,retval3;
  unsigned long long values[28]; // 3+7+9+9
  char s[64];

  retval = extract_getrusage(p->buf,p->len,values);
  p++;
  retval1 = extract_proc_task_stat(p->buf,p->len,values+retval);
  p++;
  retval2 = extract_proc_task_io(p->buf,p->len,values+retval+retval1);
  p++;
  retval3 = extract_proc_task_schedstat(p->buf,p->len,values+retval+retval1+retval2);

  LIBPAPIEX_DEBUG("%d extra fields",retval+retval1+retval2+retval3);
  for (i=0;i<retval+retval1+retval2+retval3;i++) {
    if ((no_leading_comma == 1) && (i == 0)) {
      sprintf(s,"%llu",values[i]);
    }
    else {
      sprintf(s,",%llu",values[i]);
    }
    strlcat(line,s,PAPIEX_MAX_CSV_STRLEN);
  }
  return retval+retval1+retval2+retval3;
}

static void append_caliper_data(char *fn, char *line, int len, papiex_caliper_data_t *d)
{
#ifdef HAVE_PAPI
  char s[64];
  int e;
  if (exeinfo) { // PAPI works
    sprintf(s,",%lld",d->real_cyc);
    strlcat(line,s,PAPIEX_MAX_CSV_STRLEN);
  }
  for (e=0;e<eventcnt;e++) {
    sprintf(s,",%lld",d->counters[e]);
    strlcat(line,s,PAPIEX_MAX_CSV_STRLEN);
  }
#endif
}

static int count_nonescaped_chars(char *str, int c)
{
  int i = 0;
  char *pch=strchr(str,c);
  while (pch!=NULL) {
    if ((pch == str) || (*(pch-1) != '\\'))
      i++;
    pch=strchr(pch+1,c);
  }
  return i;
}

int write_header(char *fn, int fd, char **enames, int ecnt, papiex_plugin_data_t *p, int num_plugins)
{
  int e, retval;
  char hdrline[PAPIEX_MAX_CSV_STRLEN] = "";

  strlcat(hdrline,"tags,hostname,exename,path,args,exitcode,exitsignal,pid,generation,ppid,pgid,sid,numtids,mpinumranks,tid,mpirank,start,end",PAPIEX_MAX_CSV_STRLEN);

  if ((p == NULL) || (num_plugins == 0)) {
    LIBPAPIEX_ERROR("write_header(%s,%d,%p,%d,%p,%d): plugin data is NULL or num_plugins is zero",
		    fn,fd,enames,ecnt,p,num_plugins);
    return -1;
  }
  
  // Plugins
  for (e=0;e<num_plugins;e++) {
    papiex_plugin_data_t *t = p + e;
    if ((t == NULL) || (t->fields == NULL) || (strlen(t->fields) == 0)) {
      LIBPAPIEX_ERROR("write_header(%s,%d,%p,%d,%p,%d): plugin %d, pointer %p, field pointer %p, len field %d",
		      fn,fd,enames,ecnt,p,num_plugins,
		      e,t,t->fields,(int)strlen(t->fields));
      return -1;
    }
    LIBPAPIEX_DEBUG("Adding %s to header line",t->fields);
    strlcat(hdrline,",", sizeof(hdrline));
    strlcat(hdrline,t->fields, sizeof(hdrline));
  }
  
  // Base headers, 
#ifdef HAVE_PAPI
  if (exeinfo) // PAPI works
    strlcat(hdrline,",rdtsc_duration",PAPIEX_MAX_CSV_STRLEN);
  // Event headers
  for (e=0;e<ecnt;e++) {
    strlcat(hdrline,",",PAPIEX_MAX_CSV_STRLEN);
    strlcat(hdrline,enames[e],PAPIEX_MAX_CSV_STRLEN); 
  }
#endif
  
  if (write_buffer(fn, fd, hdrline))   // Write line
    return -1;

  retval = count_nonescaped_chars(hdrline,',');
  if (retval <= 0) 
    LIBPAPIEX_ERROR("File %s header has %d separators: %s",fn,retval,hdrline);
  return(retval);   // Hdr finished, count up fields for later sanity checking
}

int write_header_csv_v2(char *fn, int fd, char **enames, int ecnt, papiex_plugin_data_t *p, int num_plugins)
{
  int e, retval;
  char hdrline[PAPIEX_MAX_CSV_STRLEN] = "";

  // the plugins and events are placed in an array with curly braces
  strlcat(hdrline, "{", PAPIEX_MAX_CSV_STRLEN);

  // Plugins
  for (e=0;e<num_plugins;e++) {
    papiex_plugin_data_t *t = p + e;
    if ((t == NULL) || (t->fields == NULL) || (strlen(t->fields) == 0)) {
      LIBPAPIEX_ERROR("write_header_csv_v2(%s,%d,%p,%d,%p,%d): plugin %d, pointer %p, field pointer %p, len field %d",
		      fn,fd,enames,ecnt,p,num_plugins,
		      e,t,t->fields,(int)strlen(t->fields));
      return -1;
    }
    // no leading comma 
    if (e != 0) {
      strlcat(hdrline,",", PAPIEX_MAX_CSV_STRLEN);
    }
    strlcat(hdrline,t->fields, PAPIEX_MAX_CSV_STRLEN);
  }
  LIBPAPIEX_DEBUG("write_header_csv_v2(%s,%d,%p,%d,%p,%d): hdrline after plugins %s",
		  fn,fd,enames,ecnt,p,num_plugins,
		  hdrline);
  
#ifdef HAVE_PAPI
  // Base headers, 
  if (exeinfo) // PAPI works
    strlcat(hdrline,",rdtsc_duration",PAPIEX_MAX_CSV_STRLEN);
  // Event headers
  for (e=0;e<ecnt;e++) {
    strlcat(hdrline,",",PAPIEX_MAX_CSV_STRLEN);
    strlcat(hdrline,enames[e],PAPIEX_MAX_CSV_STRLEN); 
  }
#endif
  strlcat(hdrline, "}", PAPIEX_MAX_CSV_STRLEN);
  LIBPAPIEX_DEBUG("write_header_csv_v2(%s,%d,%p,%d,%p,%d): hdrline after eventss %s",
		  fn,fd,enames,ecnt,p,num_plugins,
		  hdrline);
  
  strlcat(hdrline, "\ttags\thostname\texename\tpath\texitcode\texitsignal\tpid\tgeneration\tppid\tpgid\tsid\tnumtids\tstart\tend\targs", PAPIEX_MAX_CSV_STRLEN);
  LIBPAPIEX_DEBUG("write_header_csv_v2(%s,%d,%p,%d,%p,%d): hdrline after fixed headers %s",
		  fn,fd,enames,ecnt,p,num_plugins,
		  hdrline);

  if (fd != -1) {
    if (write_buffer(fn, fd, hdrline))   // Write line
      return -1;
  }
  
  retval = count_nonescaped_chars(hdrline,'\t');
  if (retval <= 0) 
    LIBPAPIEX_ERROR("File %s header has %d separators: %s",fn,retval,hdrline);

  return(retval);   // Hdr finished, count up fields for later sanity checking
}

char* papiex_escape_chars(char* buffer) {
  int i,j,l=0;
  const char esc_char[]= { '\a','\b','\f','\n','\r','\t','\v', '\\', ','};
  const char essc_str[]= {  'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', ','};
  char *dest;
  
  if ((buffer == NULL) || ((l = strlen(buffer)) == 0))
    dest = strdup("");
  else {
    dest=(char*)calloc(l*2,sizeof(char));
    char* ptr=dest;

    for(i=0;i<l;i++){
      for(j=0; j< 9 ;j++){
	if( buffer[i]==esc_char[j] ){
	  *ptr++ = '\\';
	  *ptr++ = essc_str[j];
	  break;
	}
      }
      if(j == 9 )
	*ptr++ = buffer[i];
    }
    *ptr='\0';
  }
  // LIBPAPIEX_DEBUG("%s,%d",dest,strlen(dest));
  if (dest == NULL) {
    LIBPAPIEX_ERROR("%s: malloc failed",buffer);
  } 
  return dest;
}

char* papiex_escape_chars_v2(char* buffer) {
  int i,j,l=0;
  const char esc_char[]= { '\a','\b','\f','\n','\r','\t','\v' };
  const char essc_str[]= {  'a', 'b', 'f', 'n', 'r', 't', 'v' };
  const int len_esc_chars = 7;
  char *dest;
  
  if ((buffer == NULL) || ((l = strlen(buffer)) == 0))
    dest = strdup("");
  else {
    dest=(char*)calloc(l*2,sizeof(char));
    char* ptr=dest;

    for(i=0;i<l;i++){
      for(j=0;j<len_esc_chars;j++){
	if (buffer[i]==esc_char[j]){
	  *ptr++ = '\\';
	  *ptr++ = essc_str[j];
	  break;
	}
      }
      if(j == len_esc_chars)
	*ptr++ = buffer[i];
    }
    *ptr='\0';
  }
  // LIBPAPIEX_DEBUG("%s,%d",dest,strlen(dest));
  if (dest == NULL) {
    LIBPAPIEX_ERROR("%s: malloc failed",buffer);
  } 
  return dest;
}

int papiex_write_data_csv(char *prefix, papiex_perthread_data_t *all[], int num, char *en[], int ecnt)
{
  papiex_perthread_data_t *thr = all[0];
  // header line 
  char line[PAPIEX_MAX_CSV_STRLEN];
  char fn[PATH_MAX] = "";
  int i = 0, rv, generation = 0;
  int hdr_sep_cnt = 0;
  int fd;

  LIBPAPIEX_DEBUG("%s,%p,%d,%p,%d",prefix,all,num,en,ecnt);

  // Open file and get generation number

  fd = create_process_file(prefix,fn,&generation);
  if (fd < 0)
    return -1;

  // Add standard events and other events in header: we use first thread as key for headers from plugins
  hdr_sep_cnt = write_header(fn, fd, en, ecnt, all[0]->plugin, PAPIEX_MAX_PLUGINS);
  LIBPAPIEX_DEBUG("%s: Header has %d separators",fn,hdr_sep_cnt);
#ifdef HAVE_PAPI
  if (hdr_sep_cnt != 45 + ( exeinfo ? ecnt+1 : 0)) // rdtsc is plus one
#else
  if (hdr_sep_cnt != 45) 
#endif
    LIBPAPIEX_ERROR("%s: header separator count bad of %d",fn,hdr_sep_cnt);
  // Escape global strings
  char *pt = papiex_escape_chars(process_tags);
  char *ph = papiex_escape_chars(process_hostname);
  char *pn = papiex_escape_chars(process_name);
  char *pfn = papiex_escape_chars(process_fullname);
  char *pa = papiex_escape_chars(process_args);

  // Now print all data to a string 
  rv = snprintf(line,PAPIEX_MAX_CSV_STRLEN,"%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%lld,%lld",
		pt,
		ph,
		pn,
		pfn,
		pa,
		process_exitcode,
		process_exitsignal,
		getpid(),
		generation,
		getppid(),
		getpgid(getpid()),
		getsid(getpid()),
		num,
		_papiex_mpi_numranks,
		thr->real_tid,
		thr->mpi_rank,
		thr->start,
		thr->end);
  free(pfn);
  free(pt);
  free(ph);
  free(pn);
  free(pa);
  
  if (rv >= PAPIEX_MAX_CSV_STRLEN)
    LIBPAPIEX_ERROR("Data truncation in main thread, file %s, line %d\n",fn,0);

  // Append the plugin data

  append_plugin_data(fn, line, PAPIEX_MAX_CSV_STRLEN, thr->plugin, PAPIEX_MAX_PLUGINS, 0);

  // Append the metric data

  append_caliper_data(fn, line, PAPIEX_MAX_CSV_STRLEN, &thr->data[0]);
  
  // Verify number of fields in the data vs the header, ignoring the 3 fields of name, path and args

  int line_sep_cnt = count_nonescaped_chars(line,',');
  LIBPAPIEX_DEBUG("%s: Line 0 has %d separators",fn,line_sep_cnt);
  if (line_sep_cnt != hdr_sep_cnt)
    LIBPAPIEX_ERROR("BUG: %s, line %d, %d hdr separators, %d line separators\n%s",fn,i,hdr_sep_cnt,line_sep_cnt,line);
  
  if (write_buffer(fn, fd, line)) {
    close(fd);
    return -1;
  }

  //  Now do all additional threads
   
  line[0] = '\0';
  for (i=1;i<num;i++) {
    thr = all[i];
    rv = sprintf(line,",,,,,,,,,,,,,,%d,%d,%lld,%lld",
		  thr->real_tid,
		  thr->mpi_rank,
		  thr->start,
		  thr->end);
    // if (rv >= PAPIEX_MAX_CSV_STRLEN)
    //      LIBPAPIEX_ERROR("Data truncation in %s, line %d\n",fn,i);
    
    append_plugin_data(fn, line, PAPIEX_MAX_CSV_STRLEN, thr->plugin, PAPIEX_MAX_PLUGINS, 0);
    append_caliper_data(fn, line, PAPIEX_MAX_CSV_STRLEN, &thr->data[0]);
    
    // Verify number of fields

    line_sep_cnt = count_nonescaped_chars(line,',');
    LIBPAPIEX_DEBUG("%s: Line %d has %d separators",fn,i,line_sep_cnt);
    if (line_sep_cnt != hdr_sep_cnt)
      LIBPAPIEX_ERROR("BUG: %s, line %d, %d hdr fields, %d line fields\n%s",fn,i,hdr_sep_cnt,line_sep_cnt,line);
    
    // Write line 
    
    if (write_buffer(fn, fd, line)) {
      close(fd);
      return -1;
    }
  }

  close(fd);
  return 0;
}

int papiex_write_data_csv_v2(char *prefix, papiex_perthread_data_t *all[], int num, char *en[], int ecnt)
{
  papiex_perthread_data_t *thr = NULL;
  char process_fields[PAPIEX_MAX_CSV_STRLEN];
  int line_max_len = PAPIEX_MAX_CSV_STRLEN;
  int i = 0, rv, generation = 0;
  int hdr_sep_cnt = 0;
  int fd = -1, header_fd = -1;

  if (_papiex_opt_csv_v2 == 0 ) {
    // Call old output handler
    return papiex_write_data_csv(prefix, all, num, en, ecnt);
  }
  LIBPAPIEX_DEBUG("%s,%p,%d,%p,%d",prefix,all,num,en,ecnt);

  // Malloc/escape global strings
  char *pt = papiex_escape_chars_v2(process_tags);
  char *ph = papiex_escape_chars_v2(process_hostname);
  char *pn = papiex_escape_chars_v2(process_name);
  char *pfn = papiex_escape_chars_v2(process_fullname);
  char *pa = papiex_escape_chars_v2(process_args);
  if ((pt == NULL) || (ph == NULL) || (pn == NULL) || (pfn == NULL) || (pa == NULL))
    return -1;
  // Allocate file write buffer (first attempt)
  char *line = (char *)calloc(1,line_max_len);
  if (line == NULL) {
    LIBPAPIEX_ERROR("malloc %d bytes failed for output line",line_max_len);
    return -1;
  }
  
  // Open the output file, this assumes prefix is fully unique for this
  // collection session. We can't use getsid(0) in the name because processes
  // can daemonize and change!
  //
  // generation number is always 0 for v2
  // this number represens how many 'execs' the same PID did. 
  // This will also create the path/prefix if needed.
  char csv_filename[PATH_MAX] = "";
  rv = snprintf(csv_filename,PATH_MAX,"%s%s-%s",prefix,process_hostname,"papiex.tsv");
  if (rv >= PATH_MAX) {
    LIBPAPIEX_ERROR("csv_filename plus prefix too long");
    return -1;
  }
  int attempt = 0;
  int flags = O_APPEND|O_WRONLY|O_CREAT|O_EXCL;
  LIBPAPIEX_DEBUG("open(%s) O_APPEND|O_WRONLY|O_CREAT|O_EXCL attempt %d",csv_filename,attempt);
 open_again:
  
  errno = 0;
  fd = open(csv_filename, flags, 0644);
  LIBPAPIEX_DEBUG("open(%s) returned %d, errno %s, attempt %d",csv_filename,fd,strerror(errno),attempt);
  if ((fd == -1) && (errno == ENOENT) && (attempt == 0)) {
    if (mkdir_p(prefix) == -1) // try to create path
      return -1;
    attempt++;
    goto open_again;
  }
  if ((fd == -1) && (errno == EEXIST)) {
    flags = O_APPEND|O_WRONLY;
    attempt++;
    LIBPAPIEX_DEBUG("open(%s) O_APPEND|O_WRONLY attempt %d",csv_filename,attempt);
    goto open_again;
  }
  if (fd == -1) {
    LIBPAPIEX_ERROR("open(%s): %s",csv_filename,strerror(errno));
    return -1;
  }
  LIBPAPIEX_DEBUG("csv_filename %s is file descriptor %d",csv_filename,fd);
  
  // write the header to a separate file, since we open in exclusive
  // mode, we only expect a single process will end up writing
  // the header. All the other processes should have the
  // open fail with EEXIST
  char header_filename[PATH_MAX];
  rv = snprintf(header_filename, PATH_MAX, "%s%s-%s",prefix,process_hostname,"papiex-header.tsv");
  if (rv >= PATH_MAX) {
    LIBPAPIEX_ERROR("header_filename plus prefix too long");
    return -1;
  }
    
  LIBPAPIEX_DEBUG("trying to open file %s",header_filename);
  header_fd = open(header_filename, O_WRONLY|O_CREAT|O_EXCL, 0444);
  if ((header_fd == -1) && (errno != EEXIST)) {
    LIBPAPIEX_ERROR("open(%s): %s",header_filename,strerror(errno));
    return -1;
  }
  // The -1/EEXIST case write will be skipped by write_header_csv_v2, but we still compute the header string
  // for sanity checking
  hdr_sep_cnt = write_header_csv_v2(header_filename, header_fd, en, ecnt, all[0]->plugin, PAPIEX_MAX_PLUGINS);
  if (hdr_sep_cnt <= 0) {
    LIBPAPIEX_ERROR("%s: hdr_sep_cnt has %d separators",header_filename,hdr_sep_cnt);
    return -1;
  }
    
  LIBPAPIEX_DEBUG("%s: has %d separators",header_filename,hdr_sep_cnt);
  close(header_fd);

  // Now print all process-wide data to a string, start/stop defined from thread 0
  thr = all[0];
  rv = snprintf(process_fields,PAPIEX_MAX_CSV_STRLEN,"%s\t%s\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%lld\t%lld\t%s",
		pt,
		ph,
		pn,
		pfn,
		process_exitcode,
		process_exitsignal,
		getpid(),
		generation,
		getppid(),
		getpgid(getpid()),
		getsid(getpid()),
		num,
		thr->start,
		thr->end,
		pa);
  if (rv >= PAPIEX_MAX_CSV_STRLEN) {
    LIBPAPIEX_ERROR("process_fields too long");
    return -1;
  }
  // These are no longer needed
  free(pfn);
  free(pt);
  free(ph);
  free(pn);
  free(pa);
  
 fill_line_again: // Label for when we may need to make the buffer bigger
  // Append all the thread data
  strlcat(line,"{",line_max_len);
  //  Now do all additional threads
  for (i=0;i<num;i++) {
    thr = all[i];
    const int no_leading_comma = (i == 0); // Only first one has leading 
    // Append the plugin data
    append_plugin_data(csv_filename, line, line_max_len, thr->plugin, PAPIEX_MAX_PLUGINS, no_leading_comma);
    // Append the metric data
    append_caliper_data(csv_filename, line, line_max_len, &thr->data[0]);
  }
  strlcat(line,"}\t",line_max_len);

  // append the process fields previously computed
  strlcat(line, process_fields, line_max_len);
  
  // Verify number of fields in the data vs the header
  // This works because threads have commas in their data
  int line_sep_cnt = count_nonescaped_chars(line,'\t');
  LIBPAPIEX_DEBUG("%s: has %d separators",csv_filename,line_sep_cnt);
  if (line_sep_cnt != hdr_sep_cnt) {
    LIBPAPIEX_ERROR("BUG: %s, %d hdr separators, %d line separators\n%s",csv_filename,hdr_sep_cnt,line_sep_cnt,line);
    return -1;
  }

  // Appends newline, checks for buffer overflow and writes or asks for more space.
  rv = write_buffer_v2(csv_filename, fd, line, line_max_len);
  if (rv == ENOSPC) {
    // Reallocate a bigger string and try agai
    memset(line,0,line_max_len*sizeof(char));
    free(line);
    line_max_len *= 2;
    line = (char *)calloc(1,line_max_len);
    if (line == NULL) {
      LIBPAPIEX_ERROR("malloc %d bytes failed for output line",line_max_len);
      close(fd);
      return -1;
    }
    goto fill_line_again;
  } else if (rv == -1) {
    close(fd);
    return -1;
  }
  
  close(fd);
  free(line);
  
  return 0;
}
