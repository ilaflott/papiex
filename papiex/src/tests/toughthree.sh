#!/bin/bash
unset PATH
tcl=/usr/bin/tclsh8.5
if [[ -x ${tcl} ]]; then
    ${tcl} toughthree.tcl
else
    exit 0
fi
