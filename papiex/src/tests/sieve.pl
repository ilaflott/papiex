#! /usr/bin/perl -w
use strict;

sub build_sieve {
  my $n = 0;
  my @upcoming_factors = ();
  my ($next_small_p, $sub_iter);
  my $gives_next_square = 5;
  
  return sub {
    LOOP: {
      if (not $n++) {
        return 2; # Special case
      }
      if (not defined $upcoming_factors[0]) {
        if ($n == $gives_next_square) {
          if (not defined ($sub_iter)) {
            # Be lazy to avoid an infinite loop...
            $sub_iter = build_sieve();
            $sub_iter->(); # Throw away 2
            $next_small_p = $sub_iter->();
          }
  
          push @{$upcoming_factors[$next_small_p]}, $next_small_p;
          $next_small_p = $sub_iter->();
          my $next_p2 = $next_small_p * $next_small_p;
          $gives_next_square = ($next_p2 + 1)/2;
          shift @upcoming_factors;
          redo LOOP;
        }
        else {
          shift @upcoming_factors;
          return 2*$n-1;
        }
      }
      else {
        foreach my $i (@{$upcoming_factors[0]}) {
          push @{$upcoming_factors[$i]}, $i;
        }
        shift @upcoming_factors;
        redo LOOP;
      }
    }
  }
}

# Produce as many primes as are asked for, or 100.
my $sieve = build_sieve();
print $sieve->(), "\n" for 1..(shift || 100);
