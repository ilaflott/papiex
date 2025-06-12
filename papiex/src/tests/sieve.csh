#!/bin/csh
set limit=100
seq 2 $limit | sort > input2
foreach n ( 2 `seq 3 2 $limit` )
  @ NSQ = ($n * $n)
#  echo "nsq" $NSQ "limit" $limit
  if ( $NSQ > $limit ) then
      break
  endif 
#  echo seq $NSQ $n $limit
  seq $NSQ $n $limit|sort > input3
#  echo -n "input3:"; cat input3
#  echo -n "input2:"; cat input2
  comm -23 input2 input3 > input2.new
#  echo "status:$status"
  rm -f input2
  mv input2.new input2
#  cat input2
#  break
end

echo SIEVE
cat input2|sort -n
rm -f input2.new input2 input3
