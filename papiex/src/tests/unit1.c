#include "sleeploop.h"
int main(int argc, char **argv)
{
  int i=10;
  while (--i)
    sieve(1000000);
  sleeploop(10000);
  return(0);
}
