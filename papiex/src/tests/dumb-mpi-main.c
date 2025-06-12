#include "sleeploop.h"

extern int MPI_Init(int *argc, char ***argv);
extern int MPI_Finalize();
  
int main(int argc, char **argv)
{
  MPI_Init(&argc,&argv);
  int i=10;
  while (--i)
    sieve(1000000);
  sleeploop(10000);
  MPI_Finalize();
  return(0);
}
