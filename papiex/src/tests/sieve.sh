#!/bin/bash
limit=100
sieve="$(seq 2 $limit|sort)"

for n in 2 $(seq 3 2 $limit)
do
  (( NSQ = $n * $n ))
#  echo "nsq" $NSQ "limit" $limit
  if [ $NSQ -gt $limit ]; then
      break;
  fi
#  echo seq $NSQ $n $limit
  input3=$(seq $NSQ $n $limit|sort)
#  echo "input3:"$input3
  input2=$sieve
#  echo "input2:"$input2
  sieve="$(comm -23 <(echo "$input2") <(echo "$input3"))"
#  echo "status:$?"
#  echo $sieve
#  break
done

echo SIEVE
echo "$sieve"|sort -n
