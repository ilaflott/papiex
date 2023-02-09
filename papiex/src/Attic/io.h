#define GPTL_OPT_ENV "GPTL_OPTIONS"
#define GPTL_ON_ENV "GPTL_ON"
#define GPTL_OFF_ENV "GPTL_OFF"

typedef struct {
  unsigned long long bytes;
} papiex_rdwr_t;

typedef struct {
  int flags;
  int mode;
  char *misc;
} papiex_open_t;

/* Note: SEEK_STRIDED MUST be defined as 0
 *       as we depend on the variables being
 *       memset to zero, and hence assume 
 *       a default STRIDED access pattern.
 */
#define SEEK_STRIDED 0
#define SEEK_SEQ     1
#define SEEK_RANDOM  2

typedef struct {
  unsigned long long num_calls;
  unsigned long long tot_abs_seek_bytes;
  unsigned long long tot_acc_bytes;
  unsigned long long stride; /* valid, if acc_type = SEEK_STRIDED */
  unsigned long long last_seek_pos;
  int acc_type ; /* SEEK_RANDOM, SEEK_STRIDED, SEEK_SEQ */
  int num_rewinds;
} papiex_seek_t;

typedef union {
  papiex_open_t open;
  papiex_rdwr_t rdwr;
  papiex_seek_t seek;
} papiex_specific_t;

/* For each read/write call, we aggregate statistics into bins
 * In each bin we keep the start time for the bin, the bytes
 * moved and the time taken.
 */
typedef struct {
  struct timeval bin_start_time; /* gettimeofday is used to set this */
  unsigned long long bytes;
  unsigned long long usecs;
  void *prev_bin_ptr;
  void *next_bin_ptr;
} call_bin_t;
  

typedef struct {
  unsigned long long num_calls;
  unsigned long long usecs;
  papiex_specific_t spec;
  call_bin_t *call_bin_ptr;
} papiex_percall_data_t;

#ifndef MAX_IO_CALL_TYPES
#define MAX_IO_CALL_TYPES 64
#endif

/* Cost entry for miss penalties */
typedef struct {
  const char* event;
  int min;
  int max;
  int avg;
} event_costs_t;

#define COST_STALLS_MIN 0
#define COST_STALLS_AVG 1
#define COST_STALLS_MAX 2

typedef struct {
  papiex_percall_data_t call_data[MAX_IO_CALL_TYPES];
} papiex_perdesc_data_t;

