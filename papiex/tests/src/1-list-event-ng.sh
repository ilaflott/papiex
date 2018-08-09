#!/bin/bash -e

tool="papiex"
comment="describe single software event"
options="-L PERF_COUNT_SW_CPU_CLOCK"

announce_and_clean_pretest
do_testcase

#checkfile "${exe}.${tool}.${host}.[0-9]*.1.txt" "  found output file"
#checkfile "${exe}.${tool}.${host}.[0-9]*.1.report.txt" "  found summary file"
