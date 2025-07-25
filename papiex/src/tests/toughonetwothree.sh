#!/bin/bash
unset PATH
tcl=/usr/bin/tclsh8.5
if [[ -x ${tcl} ]]; then
    echo "tclsh8.5 found"
else
    echo "tclsh8.5 not found"
    exit 1
fi

${tcl} toughone.tcl && echo "toughone success" || exit 1
${tcl} toughtwo.tcl && echo "toughtwo success" || exit 1
${tcl} toughthree.tcl && echo "toughthree success" || exit 1
