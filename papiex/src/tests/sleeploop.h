#include <unistd.h>
#include <stdio.h>

int sieve(int number)
{
  int i,j;
  int primes[number+1];

  //populating array with naturals numbers
  for(i = 2; i<=number; i++)
    primes[i] = i;

  i = 2;
  while ((i*i) <= number)
    {
      if (primes[i] != 0)
        {
	  for(j=2; j<number; j++)
            {
	      if (primes[i]*j > number)
		break;
	      else {
		// Instead of deleteing , making elemnets 0
		primes[primes[i]*j]=0;
	      }
            }
        }
      i++;
    }

  int cnt = 0;
  for(i = 2; i<=number; i++)
    {
      //If number is not 0 then it is prime
      if (primes[i]!=0)
	{ cnt++; // printf("%d\n",primes[i]); 
        }
    }

  printf("%d primes of %d found\n",cnt,number);
  return cnt;
}

int sleeploop(int us)
{
  while (--us)
    usleep(1);
  return us;
}

