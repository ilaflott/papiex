#!/bin/bash -e

exe="basic"
tool="papiex"
comment="serial, unthreaded with multiple software events"
nz_events="PERF_COUNT_SW_CPU_CLOCK PERF_COUNT_SW_TASK_CLOCK"
events="PERF_COUNT_SW_PAGE_FAULTS PERF_COUNT_SW_PAGE_FAULTS_MIN PERF_COUNT_SW_PAGE_FAULTS_MAJ PERF_COUNT_SW_CONTEXT_SWITCHES PERF_COUNT_SW_CPU_MIGRATIONS"
for e in $nz_events $events; do
    options="$options -e $e"
done

announce_and_clean_pretest
do_testcase

outf=$(checkfile "${exe}.${tool}.${host}.[0-9]*.1.txt")
findpattern "${INT_NUM_NZ}\s+\[PROCESS\] Wallclock usecs" $outf
findpattern "${INT_NUM_NZ}\s+Real usecs" $outf
findpattern "${INT_NUM_NZ}\s+Real cycles" $outf
findpattern "${INT_NUM_NZ}\s+Virtual usecs" $outf
findpattern "${INT_NUM_NZ}\s+Virtual cycles" $outf
findpattern "${INT_NUM_NZ}\s+PERF_COUNT_SW_CPU_CLOCK" $outf
findpattern "${INT_NUM_NZ}\s+PERF_COUNT_SW_TASK_CLOCK" $outf
for e in $events; do
    findpattern "${INT_NUM}\s+$e" $outf
done
#cat $outf
#findpattern "^Wallclock seconds\s+\.+\s+${SCI_NUM}" "${exe}.${tool}.${host}.[0-9]*.1.report.txt"  "  report measures wallclock"

