#!/bin/bash -e

#
# Setup
#

mytmpdir=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')
PAPIEX_OUTPUT=${mytmpdir}/
PAPIEX_OPTIONS=${PAPIEX_OPTIONS:-"COLLATED_TSV"}
papiex_prefix=${PAPIEX_PREFIX:-$(pwd)/papiex-epmt-install}
papiex_data_file=${PAPIEX_OUTPUT}`hostname`-papiex.tsv
papiex_data_header_file=${PAPIEX_OUTPUT}`hostname`-papiex-header.tsv
savedir=$(pwd)

# test results expected from image centos:7, 2023 1 Feb

kernel_version=linux-4.9.337.tar.gz
total_lines_csv=14390
total_words_csv=557554
total_lines_csv_header=1
total_words_csv_header=16

# Function that looks like a command
papiex()
{
    PAPIEX_OUTPUT=${PAPIEX_OUTPUT} PAPIEX_OPTIONS=${PAPIEX_OPTIONS} LD_PRELOAD=${papiex_prefix}/lib/libpapiex.so:${papiex_prefix}/lib/libmonitor.so $*
}

#
# Quick test
#

papiex sleep 1
ls ${papiex_data_file} > /dev/null
ls ${papiex_data_header_file} > /dev/null

#
# Linux compile test
#
export PAPIEX_TAGS=transfer
papiex curl https://cdn.kernel.org/pub/linux/kernel/v4.x/${kernel_version} > ${kernel_version}

fn=$(pwd)/${kernel_version}
cd ${PAPIEX_OUTPUT}

export PAPIEX_TAGS=uncompress
tar -xzf ${fn}

cd linux-4.9.337

export PAPIEX_TAGS=config
papiex make distclean mrproper tinyconfig > /dev/null

export PAPIEX_TAGS=compile
papiex make -j16 > /dev/null

cp ${papiex_data_file} ${papiex_data_header_file} ${savedir}

ls ${papiex_data_file} > /dev/null
ls ${papiex_data_header_file} > /dev/null
wc_results=$(wc -lw ${papiex_data_file})
wc_header_results=$(wc -lw ${papiex_data_header_file})

rm -rf ${PAPIEX_OUTPUT} ${kernel_version} ${fn}

IFS=', ' read -r -a array <<< "${wc_results}"
echo data: wc -lw ${wc_results}

# verify

echo lines ${array[0]} == ${total_lines_csv}
echo words ${array[1]} == ${total_words_csv}
[ ${array[0]} -eq ${total_lines_csv} ]
[ ${array[1]} -eq ${total_words_csv} ]

IFS=', ' read -r -a array <<< "${wc_header_results}"
echo header: wc -lw ${wc_results}
echo lines ${array[0]} == ${total_lines_csv_header}
echo words ${array[1]} == ${total_words_csv_header}
[ ${array[0]} -eq ${total_lines_csv_header} ]
[ ${array[1]} -eq ${total_words_csv_header} ]
