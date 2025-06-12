#!/bin/bash -xe

if [ ! -d papiex-epmt-install ]; then
    echo "Please run where papiex-epmt-install exists"
    exit 1
fi

#
# Setup
#
savedir=`pwd`
mytmpdir=$(mktemp -d 2>/dev/null || mktemp -d -t 'mytmpdir')
cp -Rp papiex-epmt-install ${mytmpdir}
cd ${mytmpdir}
export PAPIEX_OPTIONS=PERF_COUNT_SW_CPU_CLOCK,COLLATED_TSV
export PAPIEX_OUTPUT=${mytmpdir}
papiex_data_file=${PAPIEX_OUTPUT}/`hostname`-papiex.tsv
PAPIEX_PREFIX=${mytmpdir}/papiex-epmt-install

papiex()
{
    LD_PRELOAD=${PAPIEX_PREFIX}/lib/libpapiex.so:${PAPIEX_PREFIX}/lib/libmonitor.so $*
}

#
# Quick test
#

papiex sleep 1
ls ${papiex_data_file} > /dev/null
rm -f ${papiex_data_file}

#
# Linux compile test
#
curl http://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.9.60.tar.gz > linux-4.9.60.tar.gz
tar -xzf linux-4.9.60.tar.gz
cd linux-4.9.60
papiex make distclean mrproper tinyconfig > /dev/null
papiex make -j16 > /dev/null
ls ${papiex_data_file} > /dev/null
wc_results=$(wc -lw ${papiex_data_file})

cp ${papiex_data_file} ${savedir}
rm -rf ${mytmpdir}
IFS=', ' read -r -a array <<< "${wc_results}"
echo ${wc_results}
echo lines ${array[0]} == 13484  
echo words ${array[1]} == 480904
[ ${array[0]} -eq 13484 ]
[ ${array[1]} -eq 480904 ]

