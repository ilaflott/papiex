#!/bin/bash -e

tool="papiex"
comment="list available events"
options="-l"

announce_and_clean_pretest
do_testcase

#checkfile "${exe}.${tool}.${host}.[0-9]*.1.txt" "  found output file"
#checkfile "${exe}.${tool}.${host}.[0-9]*.1.report.txt" "  found summary file"

