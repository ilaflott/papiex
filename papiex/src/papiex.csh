#!/bin/echo Run:.

#
# Installation: . papiex.sh -or-
#               source papiex.sh
#
# This will define the papiex function, which looks like a command to your shell.
# 
# Usage:        papiex <command> 
#
# Then look for the the csv file in your $PWD
#      

if ( ! $?PAPIEX_OPTIONS ) then
    set PAPIEX_OPTIONS="PERF_COUNT_SW_CPU_CLOCK"
endif
if ( ! $?PAPIEX_INSTALL_PREFIX ) then
    set called=($_)
    set script_fn=`readlink -f $called[2]`
    set script_dir = `dirname "$script_fn"`
    set PAPIEX_INSTALL_PREFIX="$PWD/papiex-oss-install"
endif
if ( ! $?PAPIEX_OUTPUT ) then
    set PAPIEX_OUTPUT=$PWD
endif
set PAPIEX="$PAPIEX_INSTALL_PREFIX/bin/monitor-run -i $PAPIEX_INSTALL_PREFIX/lib/libpapiex.so"

command -v $PAPIEX_INSTALL_PREFIX/bin/monitor-run >&/dev/null
if ( $status != 0 ) then
    echo "Please set PAPIEX_INSTALL_PREFIX variable and rerun, monitor-run not found"
else
    setenv PAPIEX_OPTIONS "$PAPIEX_OPTIONS"
    setenv PAPIEX_INSTALL_PREFIX "$PAPIEX_INSTALL_PREFIX"
    setenv PAPIEX_OUTPUT "$PAPIEX_OUTPUT"
    alias papiex $PAPIEX \!:\*
endif
