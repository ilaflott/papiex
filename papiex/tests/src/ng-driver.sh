#!/bin/bash
#shopt -s nullglob
# set -x # debugging
# set -T (functrace) # inherits trap handlers
# usage:
# [DEBUG=1] [QUICK=1] ./ng-driver [quick]

mpi_launch="mpirun"
mpi_launch_args="-np 4"
DEBUG=${DEBUG:-0}
QUICK=${QUICK:-0}
SCI_NUM="[0-9]\.[0-9e]+-?[0-9]+"
SCI_NUM_GREATER_THAN_TEN="[0-9]\.[0-9e]+[1-9]+"
INT_NUM="[0-9][0-9]*"
INT_NUM_NONZERO="[1-9][0-9]*"
host=`hostname -s`
failed=0
passed=0
num_tests=0
#PATH="$PWD:$PATH"
tmpfile_stdout=""
tmpfile_stderr=""
# >&3 2>&4
cmdline=""

function do_testcase() {
    debug "Running ($cmdline) 2> $tmpfile_stderr > $tmpfile_stdout)"
    $cmdline 2> $tmpfile_stderr > $tmpfile_stdout
    stat=$?
    debug "Exit status was $stat"
    sync
    [ $stat -eq 0 ] || die "$cmdline returned exit code $stat"
}

function clean_tmp_files() {
    set -f
    if [ $# -ne 0 ]; then
	debug "rm -f $*"
	rm -f $*
    fi
    set +f
    rm $tmpfile_stdout
    rm $tmpfile_stderr
}

function get_tmp_files() {
    tmpfile_stdout=$(mktemp /tmp/papiex-tests-stdout.XXXXXX)
#    tail -f "$tmpfile_stdout" &
#    exec 3>"$tmpfile_stdout"
    debug "test standard out in $tmpfile_stdout on FD 3"
    tmpfile_stderr=$(mktemp /tmp/papiex-tests-stderr.XXXXXX)
#    tail -f "$tmpfile_stderr" &
#    exec 4>"$tmpfile_stderr"
    debug "test standard error in $tmpfile_stderr on FD 4"
#    rm "$tmpfile_stdout"
#    rm "$tmpfile_stderr"
# File descriptor is still open for duration of this script
}

function announce_and_clean_pretest() {
    debug "tool $tool"
    debug "exe $exe"
#    debug "options $options"
    debug "comment $comment"
    [ "$cmdline" ] || cmdline="$tool $options $exe"
#    debug "cmdline $cmdline"

    if  [ ! "$tool" ] || [ ! "$comment" ]; then
	die "Ill formed test script: $BASH_SOURCE"
    fi
#    if [ $exe == "" ] || [ $tool == "" ] ; then
#	die "Ill formed test script: $BASH_SOURCE"
#    fi
#    echo "Running $TEST:$tool:$comment"
#    rm -irf $exe*$tool*
}

function die() {
  echo $* >&2
  exit 1
}

function warn() {
  error_code=1
  echo $* >&2
}

function debug() {
  [ $DEBUG -eq 0 ] || echo "$BASH_SOURCE:$*" >&2
}  

function checkfile() {
  if [ "$#" -ne 1 ]; then
      die "checkfile() takes exactly one argument: $BASH_SOURCE"
  fi
# Turn off pattern matching
  set -f
  debug "checkfile($1)"
  set +f
# If glob doesn't match, return null
  shopt -s nullglob
  file=( $1 )
  shopt -u nullglob
  num=${#file[@]}
  debug "matched $num files (" ${file[@]} ")"
  if [ $num -gt 1 ]; then
      warn "$1 matches more than one file"
  elif [ $num -eq 0 ]; then
      warn "$1 does not exist"
  elif [ ! -r $file ]; then
      warn "$1 not readable"
  elif [ ! -s $file ]; then
      warn "$1 size not > 0"
  else
      echo $file
      return
  fi
#  return $file
}

# checkfile_real "descriptions" "pattern or singleton"

function checkfile_real () {
    desc=$1
    shift
    while (( "$#" )); do
	debug "Looking for $1"
	if [ ! -s $1 -o ! -r $1 ]; then
	    warn "$desc, $1 not found, not readable or not > size 0"
	fi
	shift
    done
    return
}

function countfiles() {
  errmsg="Could not find $2 files matching $1"
  debug "Looking for $2 files matching $1"
  num_files=$(ls -1 $1 | wc -l)
  [ $num_files -eq $2 ] || warn $errmsg
  debug "$3"
}


function checkdir() {
  errmsg="Could not find directory $1"
  debug "Looking for directory $1"
  [ -d $1 ] || warn $errmsg
  [ $DEBUG -eq 0 ] || [ "$2" == "" ] || echo "$2"
}

function findpattern() {
  if [ "$#" -ne 2 ]; then
      die "findpattern() takes exactly two arguments"
  fi
  errmsg="Could not find pattern $1 in $2"
  debug "Looking for pattern $1 in $2" 
  egrep -s -q "$1" $2 > /dev/null || warn $errmsg
}

function dump_info() {
    echo ""
    echo "*** papiex tester: export DEBUG=1 for verbose output ***"
    paranoid=`cat /proc/sys/kernel/perf_event_paranoid`
    echo "host: "$host
    echo -n "machine: "
    uname -a
    echo -n "uid: "
    id -u
    p=$((paranoid + 0)) 
    echo "perf_event_paranoid: "$p
    if [ $p -gt 2 ]; then
	echo 2 > /proc/sys/kernel/perf_event_paranoid
	if [ $? -ne 0 ]; then
	    warn "/proc/sys/kernel/perf_event_paranoid could not be changed to 2"
	else
	    echo "Changed /proc/sys/kernel/perf_event_paranoid to 2 for testing!" >&2 
	fi
    fi
    echo ""
}

dump_info
echo "SCRIPT:TARGET:COMMENT:RESULT"
for test in [0-9]*-*-ng; do
  num_tests=`expr $num_tests + 1`
  TEST=$(basename $test)
  error_code=0
  cmdline=""
  exe="papiex"
  tool=""
  options=""
  comment=""
  outf=""
  get_tmp_files
  debug "Running (. ./$test)"
  . ./$test
  if [ $? -eq 0 -a $error_code -eq 0 ]; then
    status="PASSED"
    passed=`expr $passed + 1`
  else
    status="FAILED"
    failed=`expr $failed + 1`
  fi
  clean_tmp_files $outf
  echo "$TEST:$exe:$comment:$status"

  # only run one test if quick set
  [ $QUICK -eq 0 ] || break
done

echo
[ $passed -eq 0 ] || echo "$passed of $num_tests PASSED"
[ $failed -eq 0 ] || echo "$failed of $num_tests FAILED"
