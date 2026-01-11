#!/bin/bash
set -euo pipefail

# shellcheck disable=SC2120
h () {
    # if arguments, print them
    [ $# == 0 ] || echo "$*"

  cat <<EOF
Usage: $(basename "${BASH_SOURCE[0]}") [OPTION]... <BINARY>
  run gdb
Available options:
  -h, --help        display this help and exit
  -m, --multicore   run multicore, bewarned that time won't pass unless you connect to the gdb port
EOF

    # if args, exit 1 else exit 0
    [ $# == 0 ] || exit 1
    exit 0
}

die() {
    local msg=$1
    local code=${2-1} # default exit status 1
    echo >&2 -e  "$msg"
    exit "$code"
}

# getopt short options go together, long options have commas
TEMP=$(getopt -o hm --long help,multicore -n "$0" -- "$@")
#shellcheck disable=SC2181
if [ $? != 0 ] ; then
    die "something wrong with getopt"
fi
eval set -- "$TEMP"

multicore=false
while true ; do
    case "$1" in
        -h|--help) h ;;
        -m|--multicore) multicore=true; shift ;;
        --) shift ; break ;;
        *) die "issue parsing args, unexpected argument '$1'!" ;;
    esac
done

binary="${1:-}"
if [[ -z "$binary" ]]; then
    h "need to pass in binary"
fi

if [[ "$multicore" == true ]]; then
    # multicore means multiple gdb per core
    # this runs gdb over a pipe for core 0, and puts conre 1 on port 3334
    gdb-multiarch --quiet -ex "target extended-remote | openocd -c 'gdb_port pipe' -f interface/cmsis-dap.cfg -c'transport select swd' -c 'adapter speed 5000' -f target/rp2040.cfg -c 'rp2040.core1 configure -gdb-port 3334'"  -x gdbConfig "$binary"
else
    # https://github.com/raspberrypi/debugprobe/issues/45#issuecomment-1332610961
    # The default configuration for openocd 0.12rc2 with an rp2040 target will debug both cores with dedicated gdb instances.
    # ...
    # The rp2040 timer is set up by default to stop counting when any core is stopped by a debugger and so the timer would never advance.
    gdb-multiarch --quiet -ex "target extended-remote | openocd -c 'gdb_port pipe' -f interface/cmsis-dap.cfg -c'transport select swd' -c 'adapter speed 5000' -c 'set USE_CORE 0' -f target/rp2040.cfg"  -x gdbConfig "$binary"
fi
