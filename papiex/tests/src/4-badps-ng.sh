#!/bin/bash -e

exe="badps"
tool="papiex"
comment="bash script that uses ps with single software event"
nz_events="PERF_COUNT_SW_CPU_CLOCK"
events=""
for e in $nz_events $events; do
    options="$options -e $e"
done

announce_and_clean_pretest
do_testcase

#outf=$(checkfile "${exe}.${tool}.${host}.[0-9]*.1.txt")
#findpattern "${INT_NUM_NZ}\s+\[PROCESS\] Wallclock usecs" $outf
#findpattern "${INT_NUM_NZ}\s+Real usecs" $outf
#findpattern "${INT_NUM_NZ}\s+Real cycles" $outf
#findpattern "${INT_NUM_NZ}\s+Virtual usecs" $outf
#findpattern "${INT_NUM_NZ}\s+Virtual cycles" $outf
#findpattern "${INT_NUM_NZ}\s+PERF_COUNT_SW_CPU_CLOCK" $outf
#
#
#cat ${exe}.${tool}.${host}.[0-9]*.1.txt
#findpattern "^Wallclock seconds\s+\.+\s+${SCI_NUM}" "${exe}.${tool}.${host}.[0-9]*.1.report.txt"  "  report measures wallclock"

