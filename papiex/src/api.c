#include "papiex_internal.h"

void papiex_start(int point, char *label)
{
  if (eventcnt <= 0)
    return; 

  LIBPAPIEX_DEBUG("START POINT %d LABEL %s",point,label);

  if ((point < 0) || (point >= thr_data->max_caliper_entries))
    {
      LIBPAPIEX_ERROR("Caliper point %d is out of range, max %d",point,thr_data->max_caliper_entries);
      return;
    }
  if (thr_data->data[point].depth)
    {
      LIBPAPIEX_ERROR("Caliper point %d is already in use",point);
      return;
    }
  if (label)
    {
      strncpy(thr_data->data[point].label, label, PAPIEX_MAX_LABEL_SIZE);
      thr_data->data[point].label[PAPIEX_MAX_LABEL_SIZE-1] = '\0';
    }

#ifdef HAVE_PAPI
  LIBPAPIEX_DEBUG("Calling PAPI_read_ts");
  int retval = PAPI_read_ts(thr_data->papi_eventset,thr_data->data[point].tmp_counters,&thr_data->data[point].tmp_real_cyc);
  if (retval != PAPI_OK) 
    {
      LIBPAPIEX_PAPI_ERROR("PAPI_read_ts",retval);
      thr_data->data[point].label[0] = '\0';
      return;
    }
  if (_papiex_opt_debug) {
    int i;
    LIBPAPIEX_DEBUG("point %d real_cyc %llu",point,thr_data->data[point].tmp_real_cyc);
    for (i=0;i<eventcnt;i++)
      LIBPAPIEX_DEBUG("point %d event %d count %llu",point,i,thr_data->data[point].tmp_counters[i]);
  }
#endif
  
  /* Update the max caliper point used */
  thr_data->data[point].depth++;
  if (thr_data->max_caliper_used <= point)
    thr_data->max_caliper_used = point;

  LIBPAPIEX_DEBUG("START POINT %d USED %d DEPTH %d",point,(int)thr_data->data[point].used,thr_data->data[point].depth);
}

void papiex_stop(int point)
{
  if (eventcnt <= 0)
    return; 

  LIBPAPIEX_DEBUG("STOP POINT %d",point);

  if ((point < 0) || (point >= thr_data->max_caliper_entries)) {
    LIBPAPIEX_ERROR("Caliper point %d out of range, max %d",point,thr_data->max_caliper_entries);
    return;
  }
  if (thr_data->data[point].depth == 0) {
    LIBPAPIEX_ERROR("Caliper point %d is not in use",point);
    return;
  }

#ifdef HAVE_PAPI
  int i, retval;
  long long counters[PAPIEX_MAX_COUNTERS], real_cyc = 0;
  LIBPAPIEX_DEBUG("Calling PAPI_read_ts");
  retval = PAPI_read_ts(thr_data->papi_eventset,counters,&real_cyc);
  // This case uses DEBUG for error messages as this may fail due to target clobbering PAPI state, like closing it's FD 
  // See comment below as well
  if (retval != PAPI_OK) {
    LIBPAPIEX_DEBUG("PAPI_read_ts(pid %d,tid %d,exe %s) failed in papiex_stop(%d): %s",(int)getpid(),(int)syscall(SYS_gettid),process_name,point,PAPI_strerror(retval));
    return;
  }
  /* Note that real_cyc for this point will be negative if PAPI fails for some reason. This is
     the way we detect if the PAPI metrics have errored out, rather than maintaining some other
     state */
  thr_data->data[point].real_cyc += real_cyc - thr_data->data[point].tmp_real_cyc;
  LIBPAPIEX_DEBUG("point %d real_cyc %llu total %llu",point,real_cyc,thr_data->data[point].real_cyc);
  for (i=0;i<eventcnt;i++) {
    thr_data->data[point].counters[i] += counters[i] - thr_data->data[point].tmp_counters[i];
    LIBPAPIEX_DEBUG("point %d event %d count %llu total %llu",point,i,counters[i], thr_data->data[point].counters[i]);
  }
#endif

  thr_data->data[point].depth--;
  thr_data->data[point].used++;

  LIBPAPIEX_DEBUG("STOP POINT %d USED %d DEPTH %d",point,(int)thr_data->data[point].used,thr_data->data[point].depth);
}

void papiex_accum(int point)
{
  /* Accum can't be called on point 0 */
  if ((point <= 0) || (point >= thr_data->max_caliper_entries))
    {
      LIBPAPIEX_DEBUG("Caliper point %d is out of range, max %d",point,thr_data->max_caliper_entries);
      return;
    }
  if (thr_data->data[point].depth == 0)
    {
      LIBPAPIEX_ERROR("Caliper point %d is not in use",point);
      return;
    }
#ifdef HAVE_PAPI
  int i, retval;
  long long counters[PAPIEX_MAX_COUNTERS], real_cyc;

  if (eventcnt <= 0)
    return; 

  LIBPAPIEX_DEBUG("ACCUM POINT %d",point);
  
  LIBPAPIEX_DEBUG("Calling PAPI_read_ts");
  retval = PAPI_read_ts(thr_data->papi_eventset,counters,&real_cyc);
  if (retval != PAPI_OK) 
    {
      LIBPAPIEX_PAPI_ERROR("PAPI_read_ts",retval);
      return;
    }
  thr_data->data[point].real_cyc += real_cyc - thr_data->data[point].tmp_real_cyc;
  thr_data->data[point].tmp_real_cyc = real_cyc;
  LIBPAPIEX_DEBUG("point %d real_cyc %llu",point,real_cyc);
  for (i=0;i<eventcnt;i++)
    {
      LIBPAPIEX_DEBUG("point %d event %d count %llu",point,i,counters[i]);
      thr_data->data[point].counters[i] += counters[i] - thr_data->data[point].tmp_counters[i];
      thr_data->data[point].tmp_counters[i] = counters[i];
    }
  LIBPAPIEX_DEBUG("POINT %d USED %d DEPTH %d",point,(int) thr_data->data[point].used,thr_data->data[point].depth);
#endif
  thr_data->data[point].used++;
}

void papiex_start__(int *point, char *label)
{
  papiex_start(*point, label);
}
void papiex_start_(int *point, char *label)
{
  papiex_start(*point, label);
}
void PAPIEX_START__(int *point, char *label)
{
  papiex_start(*point, label);
}
void PAPIEX_START_(int *point, char *label)
{
  papiex_start(*point, label);
}
void papiex_accum__(int *point)
{
  papiex_accum(*point);
}
void papiex_accum_(int *point)
{
  papiex_accum(*point);
}
void PAPIEX_ACCUM__(int *point)
{
  papiex_accum(*point);
}
void PAPIEX_ACCUM_(int *point)
{
  papiex_accum(*point);
}
void papiex_stop__(int *point)
{
  papiex_stop(*point);
}
void papiex_stop_(int *point)
{
  papiex_stop(*point);
}
void PAPIEX_STOP__(int *point)
{
  papiex_stop(*point);
}
void PAPIEX_STOP_(int *point)
{
  papiex_stop(*point);
}
