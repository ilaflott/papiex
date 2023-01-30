void __f () { /* Do something. */; }
void monitor_set_size_rank(int size, int rank) __attribute__ ((weak, alias ("__f")));

int MPI_Init(int *argc, char ***argv)
{
  extern void monitor_set_size_rank(int size, int rank);
  monitor_set_size_rank(4,1);
  return(0);
}
int MPI_Finalize()
{
  return(0);
}

