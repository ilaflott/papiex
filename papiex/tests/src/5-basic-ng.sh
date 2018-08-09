#!/bin/bash -e

exe="basic"
tool="papiex"
comment="serial, unthreaded with the default set of multiple software events"
nz_events="PERF_COUNT_SW_CPU_CLOCK PERF_COUNT_SW_TASK_CLOCK"
events=""
options=""

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
#cat $outf
#findpattern "^Wallclock seconds\s+\.+\s+${SCI_NUM}" "${exe}.${tool}.${host}.[0-9]*.1.report.txt"  "  report measures wallclock"

