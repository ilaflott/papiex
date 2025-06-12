#!/bin/tcsh

if ($?MODULESHOME) then       
  if ("$MODULESHOME" == "")  then
    set TMH=/usr/share/modules
  else
    set TMH=$MODULESHOME
  endif
else
    set TMH=/usr/share/modules
endif

if ( ! -d $TMH ) then
    echo "$TMH not found. Please set MODULESHOME variable"
    exit 1
endif

if ( ! -r $TMH/init/tcsh ) then
    echo "$TMH/init/tcsh not found. Please set MODULESHOME variable"
    exit 1
endif

# Load the module command
source $TMH/init/tcsh

module load ./papiex-test-module

# This will error if undefined
echo "PAPIEX_DUMMY is [$PAPIEX_DUMMY]"
echo "PATH is [$PATH]"
echo ""
echo "Now sleeping for 1..."
sleep 1
exit 0
