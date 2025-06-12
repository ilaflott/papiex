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

PAPIEX_OPTIONS=${PAPIEX_OPTIONS:-PERF_COUNT_SW_CPU_CLOCK}
PAPIEX_OUTPUT=${PAPIEX_OUTPUT:-$PWD}
if [ "$PAPIEX_INSTALL_PREFIX" == "" ]; then
    PAPIEX_SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    PAPIEX_INSTALL_PREFIX=${PAPIEX_INSTALL_PREFIX:-$PAPIEX_SCRIPTDIR/../..}
fi
command -v $PAPIEX_INSTALL_PREFIX/bin/monitor-run 2>&1 > /dev/null
if [ $? -ne 0 ]; then
    echo "Please set PAPIEX_INSTALL_PREFIX variable and rerun, monitor-run not found."
    return
fi

papiex() {
    PAPIEX_OPTIONS=${PAPIEX_OPTIONS} PAPIEX_OUTPUT=${PAPIEX_OUTPUT} $PAPIEX_INSTALL_PREFIX/bin/monitor-run -i $PAPIEX_INSTALL_PREFIX/lib/libpapiex.so $*
}
