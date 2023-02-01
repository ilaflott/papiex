#!/bin/bash
#set -uo pipefail
#set -x

RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[0;37m'
NC='\033[0m'

TESTSDIR=`dirname ${BASH_SOURCE}`
# Convert to absolute path
TESTSDIR="$(cd "$(dirname "$TESTSDIR")"; pwd)/$(basename "$TESTSDIR")"

# Convert to absolute path
PAPIEX_PREFIX="${PAPIEX_PREFIX:-${TESTSDIR}/../../../papiex-oss-install}"
PAPIEX_PREFIX="$(cd "$(dirname "${PAPIEX_PREFIX}")"; pwd)/$(basename "${PAPIEX_PREFIX}")"

if [ ! -d ${PAPIEX_PREFIX} ]; then
    echo -e "${RED}PAPIEX_PREFIX of ${PAPIEX_PREFIX} does not exist or is not readable.${NC}"
    exit 1
fi

PAPIEX_OPTIONS=${PAPIEX_OPTIONS:-COLLATED_TSV,PERF_COUNT_SW_CPU_CLOCK}

PAPIEX_OUTPUT="/tmp"
if [ ! -d ${PAPIEX_OUTPUT} ]; then
    echo -e "${RED}${PAPIEX_OUTPUT} does not exist or is not readable.${NC}"
    exit 1
fi
TEST_OUTPUT_DATADIR="${TEST_OUTPUT_DATADIR:-${PAPIEX_PREFIX}/tmp}"
if [[ ! $PAPIEX_OPTIONS =~ COLLATED_TSV ]] ; then
    TMP_OUTPUT_PATTERN="${PAPIEX_OUTPUT}/*-papiex-*.csv"
    rm -f $TMP_OUTPUT_PATTERN
else
    TMP_OUTPUT_PATTERN="${PAPIEX_OUTPUT}/*-papiex.tsv"
    TMP_OUTPUT_HEADER_PATTERN="${PAPIEX_OUTPUT}/*-papiex-header.tsv"
    rm -f $TMP_OUTPUT_PATTERN $TMP_OUTPUT_HEADER_PATTERN
fi

if [ ! -z ${LD_LIBRARY_PATH+x} ]; then
    export LD_LIBRARY_PATH=${PAPIEX_PREFIX}/lib:$LD_LIBRARY_PATH
else
    export LD_LIBRARY_PATH=${PAPIEX_PREFIX}/lib
fi

if [ ! -d ${PAPIEX_PREFIX}/lib ]; then
    echo -e "${RED}${PAPIEX_PREFIX}/lib does not exist or is not readable.${NC}"
    exit 1
fi

preload_libraries=${PAPIEX_PREFIX}/lib/libpapiex.so:${PAPIEX_PREFIX}/lib/libmonitor.so
PAPIEX="PAPIEX_OUTPUT=${PAPIEX_OUTPUT} PAPIEX_OPTIONS=${PAPIEX_OPTIONS} LD_PRELOAD=$preload_libraries"

no_papi=0
err=0
skipped=0
passed=0
expected=0


#    export PAPIEX_DEBUG=1
#    export MONITOR_DEBUG=1

declare -a modules=(
    "tcsh -f module-test.csh"
)
declare -a simple=(
    "sleep 1"
#    "sleep 10 & sleep 1; kill -s SIGINT \$!"
    "ps -fade"
    "host google.com"
#    "echo : | tr ':' '\n'"
    "sed -e s/,//g"
)
declare -a compiles=(
    "gcc -Wall unit1.c -o unit1a"
    "gcc -Wall -fPIC -shared dumb-mpi.c -o dumb-mpi.so"
    "gcc -Wall dumb-mpi-main.c ./dumb-mpi.so -o dumb-mpi"
    "gcc -Wall unit1.c -o unit1a"
    "gcc -pthread dotprod_mutex.c -o dotprod_mutex"
    "gcc -pthread dotprod_mutex_fork.c -o dotprod_mutex_fork"
    "g++ -fopenmp md_openmp.cpp -o md_openmp"
    "gfortran -fopenmp fft_openmp.f90 -o fft_openmp"
)
declare -a binaries=(
    "./unit1a"
    "./dotprod_mutex"
    "./dotprod_mutex_fork"
    "./md_openmp"
    "./fft_openmp"
    "./dumb-mpi"
)
declare -a shells=(
    "bash --noprofile -c 'sleep 1'"
    "tcsh -f -c 'sleep 1'"
    "csh -f -c 'sleep 1'"
    "tcsh -f evilcsh.csh"
    "csh -f evilcsh.csh"
    "tclsh toughone.tcl"
    "tclsh toughtwo.tcl"
    "bash toughthree.sh"
    "git clone https://github.com/NOAA-GFDL/mkmf.git"
    "rm -rf mkmf"
)
declare -a sieves=(
    "bash --noprofile sieve.sh 100"
    "tcsh -f sieve.csh 100"
    "csh -f sieve.csh 100" 
    "python2 sieve.py"
    "python3 sieve.py"
    "perl sieve.pl"
)
declare -a sshtests=(
    "ssh -o PreferredAuthentications=publickey localhost /bin/true"
    "ssh -o PreferredAuthentications=publickey cshsucks@localhost /bin/true"
    "ssh -o PreferredAuthentications=publickey tcshsucks@localhost /bin/true"
)

function papiex()
{
    #    echo $#,"($*)"
    TORUN=$*
    #    read -r -a TORUN_ARRAY <<< "${TORUN}"
    #    echo "TO RUN LINE:(${TORUN})"
    echo -n "${TORUN}"
    #    command -v "${TORUN_ARRAY[0]}" >/dev/null 2>&1
    command -v ${TORUN} > /dev/null 2>&1
    if [ $? -ne 0 ]; then
	echo -e ": ${RED}SKIPPED${NC} ${YELLOW}executable not found${NC}"
	((++skipped))
	return 0
    fi
    eval ${PAPIEX[@]} ${TORUN} >> /tmp/stdout 2>> /tmp/stderr < /dev/null
    stat=$?
    if [ $stat -ne 0 ]; then
	echo -n -e ": ${RED}FAIL($stat) ${NC}[${YELLOW}"; tail -n 1 /tmp/stderr | tr -d '\n' ; echo -e "${NC}]"
	((++err))
	return $stat
    fi

    #
    # Much better testing here
    #
    errhere=0
    papiex_files=( $(ls ${TMP_OUTPUT_PATTERN} 2>/dev/null) )
    if [ $? -ne 0 ]; then
	echo -e ": ${RED}FAIL(no output files found)${NC}"
	((++err))
	((++errhere))
    fi

    if [[ ! $PAPIEX_OPTIONS =~ COLLATED_TSV ]] ; then
	for f in "${papiex_files[@]}"
	do
	    mv $f ${TEST_OUTPUT_DATADIR}
	    f=${TEST_OUTPUT_DATADIR}/`basename $f`

	    # Check size
	    if [ ! -s $f ]; then
		echo -e ": ${RED}FAIL(size not zero)${NC}"
		((++err))
		((++errhere))
		continue
	    fi
	    if [[ $tsv_check -eq 0 ]]; then 
		# Check lines
		l=$(($(cat $f | wc -l)))
		if [[ $? -ne 0 || $l -lt 2 ]]; then
		    echo -e ": ${RED}FAIL(at least 2 lines)${NC}"
		    ((++err))
		    ((++errhere))
		    continue
		fi
	    fi
	    # Check headers
	    cols=$(head -n 2 $f | tail -n 1)
	    if [[ $? -ne 0 ]]; then
		echo -e ": ${RED}FAIL(columns row exists)${NC}"
		((++err))
		((++errhere))
		continue
	    fi
	    # Check number of unescaped commas in first and second row
	    hdrcols=$(head -n 1 $f | tr -d -c ',\n' | awk '{ print length; }')
	    rowcols=$(head -n 2 $f | tail -n 1 | sed -e "s/\\\,//g" | tr -d -c ',\n' | awk '{ print length; }')
	    if [[ $hdrcols != $rowcols ]]; then
		echo -e ": ${RED}FAIL($hdrcols columns in header vs $rowcols in data)${NC}"
		((++err))
		((++errhere))
		continue
	    fi
	    # Check for valid (positive) numbers in number fields
	    reason=`${TESTSDIR}/checkcsv.py $f 2>&1`
	    if [ $? -ne 0 ]; then
		echo -e ": ${RED}FAIL($reason)${NC}"
		((++err))
		((++errhere))
		continue
	    fi
	done
    else
	if [ ${#papiex_files[@]} -gt 1 ]; then
	    echo -e ": ${RED}FAIL(more than one output file found)${NC}"
	    ((++err))
	    ((++errhere))
	fi
	papiex_header_files=( $(ls ${TMP_OUTPUT_HEADER_PATTERN} 2>/dev/null) )
	if [ $? -ne 0 ]; then
	    echo -e ": ${RED}FAIL(no output header files found)${NC}"
	    ((++err))
	    ((++errhere))
	fi
	if [ ${#papiex_header_files[@]} -gt 1 ]; then
	    echo -e ": ${RED}FAIL(more than one output header file found)${NC}"
	    ((++err))
	    ((++errhere))
	fi

	f="${papiex_files[@]}"
	fh="${papiex_header_files[@]}"

	# Check size
	if [ ! -s $f ]; then
	    echo -e ": ${RED}FAIL(data file size not zero)${NC}"
	    ((++err))
	    ((++errhere))
	fi
	# Check lines
	l=$(($(cat $f | wc -l)))
	if [[ $? -ne 0 || $l -lt 1 ]]; then
	    echo -e ": ${RED}FAIL(data file at least 1 lines)${NC}"
	    ((++err))
	    ((++errhere))
	    continue
	fi
	# Check size header
	if [ ! -s $fh ]; then
	    echo -e ": ${RED}FAIL(header file size not zero)${NC}"
	    ((++err))
	    ((++errhere))
	fi
	# Check lines header
	l=$(($(cat $fh | wc -l)))
	if [[ $? -ne 0 || $l -ne 1 ]]; then
	    echo -e ": ${RED}FAIL(header file not 1 line)${NC}"
	    ((++err))
	    ((++errhere))
	    continue
	fi

	# Check number of unescaped tabs in first row of header and data file
	hdrcols=$(head -n 1 $fh | tr -d -c '\t\n' | awk '{ print length; }')
	rowcols=$(head -n 1 $f  | tr -d -c '\t\n' | awk '{ print length; }')
	if [[ $hdrcols != $rowcols ]]; then
	    echo -e ": ${RED}FAIL($hdrcols columns in header vs $rowcols in data)${NC}"
	    exit 0
	    ((++err))
	    ((++errhere))
	    continue
	fi

	# Check for valid (positive) numbers in number fields
	reason=`${TESTSDIR}/checkcsv.py $f 2>&1`
	if [ $? -ne 0 ]; then
	    echo -e ": ${RED}FAIL($reason)${NC}"
	    ((++err))
	    ((++errhere))
	    continue
	fi
    fi
    
    if [[ $errhere == 0 ]]; then
	pass
    fi
    return $stat
}

function test_tags()
{
    for i in "${compiles[@]}"
    do
	export PAPIEX_TAGS=compiles
	papiex $i
    done
    for i in "${binaries[@]}"
    do
	export PAPIEX_TAGS=binaries
	papiex $i
    done
    for i in "${simple[@]}"
    do
	export PAPIEX_TAGS=simple
	papiex "$i"
    done
    for i in "${sieves[@]}"
    do
	export PAPIEX_TAGS=sieves
	papiex "$i"
    done
# not working
#    for i in "${modules[@]}"
#    do
#	export PAPIEX_TAGS=modules
#	papiex "$i"
#    done
    for i in "${shells[@]}"
    do
	export PAPIEX_TAGS=shells
	papiex "$i"
    done
    for i in "${sshtests[@]}"
    do
	# We test the ssh command before running papiex because native testing won't have the proper goods
	$i >/dev/null 2>&1 
	if [ $? -ne 0 ]; then
	    echo -n "$i"
	    echo -e ": ${RED}SKIPPED${NC} ${YELLOW}ssh failed, keys missing${NC}"
	    ((++skipped))
	else
	    export PAPIEX_TAGS=sshtests
	    papiex "$i"
	    if [[ ! ${no_papi} ]]; then
		# We expect ssh to fail the papi counts because it closes FD's
		((++expected))
	    fi
	fi
    done
}

papiex_output_files=
tmp_datadir=/tmp

get_papiex_output_files() 
{
    papiex_output_files=( $(ls ${tmp_datadir}/*papiex*.csv 2>/dev/null) )
    if [ $? -ne 0 ]; then
	echo -e "${RED}FAIL(no output files found in ${tmp_datadir})${NC}"
	((++err))
	return 1
    fi
    return 0
}

check_papiex_output_file()
{
    f=$1
	# Check size
	if [ ! -s $f ]; then
	    echo -e "${RED}FAIL(${f} size not zero)${NC}"
	    ((++err))
            return 1
	fi
	# Check lines
	l=$(($(cat $f | wc -l)))
	if [[ $? -ne 0 || $l -lt 2 ]]; then
	    echo -e "${RED}FAIL(${f} at least 2 lines)${NC}"
	    ((++err))
            return 1
	fi
	# Check headers
	cols=$(head -n 2 $f | tail -n 1)
	if [[ $? -ne 0 ]]; then
	    echo -e "${RED}FAIL(${f} columns row exists)${NC}"
	    ((++err))
            return 1
	fi
	# Check number of unescaped commas in first and second row
	hdrcols=$(head -n 1 $f | tr -d -c ',\n' | awk '{ print length; }')
	rowcols=$(head -n 2 $f | tail -n 1 | sed -e "s/\\\,//g" | tr -d -c ',\n' | awk '{ print length; }')
	if [[ $hdrcols != $rowcols ]]; then
	    echo -e "${RED}FAIL(${f} $hdrcols columns in header vs $rowcols in data)${NC}"
	    ((++err))
            return 1
	fi
	# Check for valid (0 or positive) numbers in number fields
	reason=`./checkcsv.py $f 2>&1`
	if [ $? -ne 0 ]; then
	    echo -e "${RED}FAIL(${f} $reason)${NC}"
	    ((++err))
            return 1
	fi
    return 0
}

check_papiex_output()
{
    RED='\033[0;31m'
    BLUE='\033[0;34m'
    YELLOW='\033[0;37m'
    NC='\033[0m'
    errhere=0

    tmp_datadir=${PAPIEX_OUTPUT}
    output_datadir=${TEST_OUTPUT_DATADIR}

    get_papiex_output_files 
    if [ $? -ne 0 ]; then
        ((++errhere))
	return 1
    fi
    papiex_files=( $papiex_output_files )

    for f in "${papiex_files[@]}"
    do
	if [[ $tmp_datadir != $output_datadir ]]; then
	    mv $f ${output_datadir}
	fi
	f=${output_datadir}/`basename $f`
        check_papiex_output_file $f
        if [ $? -ne 0 ]; then
            ((++errhere))
        fi
    done
    
    return $errhere
}

papiex_fn()
{
    rm -f ${TMP_OUTPUT_PATTERN}
    echo -n "$1"
    PAPIEX_OPTIONS=${PAPIEX_OPTIONS} PAPIEX_OUTPUT=${PAPIEX_OUTPUT} LD_PRELOAD=${preload_libraries} "$1" >> /tmp/stdout 2>> /tmp/stderr < /dev/null
}

pass()
{
    echo -e ": ${BLUE}PASS${NC}"
    ((++passed))
}

do_nametests()
{
    echo "int main(){}" > noop.c
    gcc noop.c -o noop
    papiex_fn ./noop
    check_papiex_output
    if [ $? -eq 0 ]; then
	    pass
    fi
    chars_to_test=( " x" , )
    for f in "${chars_to_test[@]}" 
    do
	name="_noop${f}"
	cp noop "${name}"
	papiex_fn ./"${name}"
	if [ $? -eq 0 ]; then
	    pass
	fi
    done
}

do_signaltests()
{
    rm -f ${TMP_OUTPUT_PATTERN}
    echo -n "/bin/sleep 10" 
    exec 2>/dev/null
    PAPIEX_OPTIONS=${PAPIEX_OPTIONS} PAPIEX_OUTPUT=${PAPIEX_OUTPUT} LD_PRELOAD=${preload_libraries} /bin/sleep 10 &
    pid=$!
    sleep 1
    kill -15 $pid >/dev/null
    wait $pid 
    exec 2>&1

    get_papiex_output_files 
    if [ $? -ne 0 ]; then
        ((++errhere))
	return 1
    fi
    papiex_files=( $papiex_output_files )

    for f in "${papiex_files[@]}"
    do
	if [[ $tmp_datadir != $output_datadir ]]; then
	    mv $f ${output_datadir}
	fi
	f=${output_datadir}/`basename $f`
        check_papiex_output_file $f
        if [ $? -ne 0 ]; then
            ((++errhere))
	    return 1
        fi
	reason=`./checkcsv.py $f exitsignal 1 2>&1`
	if [ $? -ne 0 ]; then
	    echo -e "${RED}FAIL(${f} $reason)${NC}"
	    ((++err))
            return 1
	fi
    done

    pass
}

function check_papi_is_working()
{
    echo "-- Checking PAPI/perf_event --"
    echo -n "PAPI:"
    if [[ ! -x ${PAPIEX_PREFIX}/bin/papi_command_line ]]; then
	# We are not built with PAPI, as the binaries are not there
	echo -e " ${RED}not compiled in${NC}"
	no_papi=1
	return
    fi
    echo -e " ${BLUE}OK${NC}"
    
    echo -n "sysctl kernel.perf_event_paranoid = "
    a=$(sysctl -n kernel.perf_event_paranoid)
    echo -n $a
    if [[ $a -eq 3 ]]; then
	echo -e ": ${RED}perf_event metrics not enabled in kernel${NC}"
	return
    fi
    echo -e " ${BLUE}OK${NC}"

    for i in $(echo ${PAPIEX_OPTIONS} | tr "," " "); do
	if [[ $i == "COLLATED_TSV" ]] ; then
	    continue
	fi
	echo -n "papi_command_line $i" 
	out=`${PAPIEX_PREFIX}/bin/papi_command_line $i`
	stat=$?
	if [[ $stat -ne 0 ]]; then
	    echo -e ": ${RED}PAPI event not working${NC}"
	    return
	fi
	val=`echo ${out} | grep -Eo "${i}\s+:\s+[0-9]+"`
	stat=$?
	if [[ $stat -ne 0 ]]; then
	    echo -e ": ${RED}PAPI event not working${NC}"
	    return
	fi
	echo -e " ${BLUE}OK${NC}"
    done
}

# MAIN
rm -rf ${TEST_OUTPUT_DATADIR}
mkdir -p ${TEST_OUTPUT_DATADIR}
if [ $? -ne 0 ]; then
    exit 1
fi
cd $TESTSDIR
#
echo "Test dir: ${TESTSDIR}"
echo "Temp output pattern: ${TMP_OUTPUT_PATTERN}"
echo "Test output data dir: ${TEST_OUTPUT_DATADIR}"
echo "Environment: PAPIEX_PREFIX=${PAPIEX_PREFIX}"
echo "Environment: PAPIEX_OPTIONS=${PAPIEX_OPTIONS}"
check_papi_is_working
#echo "Invocation: ${PAPIEX}"
echo "-- Test Data --"
test_tags
if [[ ! $PAPIEX_OPTIONS =~ COLLATED_TSV ]] ; then
# not yet implemented in TSV
    do_nametests
    do_signaltests
fi
echo "-- Test Results --"
echo " PASSED: $passed"
echo "SKIPPED: $skipped"
echo " FAILED: $err ($expected expected)"
echo 
if [[ $err -gt 0 && $err -ne $expected ]]; then
    echo "FAILED"
    ls -l /tmp/stdout
    ls -l /tmp/stderr
    echo "head of stderr is"
    head /tmp/stderr
    exit 1
else
    echo "PASSED"
fi
