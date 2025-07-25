#!/bin/bash
unset PATH
tcl=/usr/bin/tclsh8.5
if [[ -x ${tcl} ]]; then
    ${tcl} toughone.tcl
    ${tcl} toughtwo.tcl
    ${tcl} toughthree.tcl
else
    echo "AM I SKIPPING BY EXITING 0 (FAUX SUCCESS) or AM I GOOD?"
    exit 0
fi
