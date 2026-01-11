#!/bin/bash
set -euo pipefail
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

vid=2e8a
pid=a

# shellcheck disable=SC2120
h () {
    # if arguments, print them
    [ $# == 0 ] || echo "$*"

  cat <<EOF
Usage: $(basename "${BASH_SOURCE[0]}") [OPTION]... <BINARY>
  flash code to board
Available options:
  -h, --help        display this help and exit
  -b, --rebuild     run cmake --build build first
  -p, --programmer  use programmer not picotool
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
TEMP=$(getopt -o hbp --long help,rebuild,programmer -n "$0" -- "$@")
#shellcheck disable=SC2181
if [ $? != 0 ] ; then
    die "something wrong with getopt"
fi
eval set -- "$TEMP"

programmer=false
rebuild=false
while true ; do
    case "$1" in
        -h|--help) h ;;
        -b|--rebuild) rebuild=true; shift ;;
        -p|--programmer) programmer=true; shift ;;
        --) shift ; break ;;
        *) die "issue parsing args, unexpected argument '$1'!" ;;
    esac
done

binary="${1:-}"
if [[ -z "$binary" ]]; then
    h "need to pass in binary"
fi

if [[ "$rebuild" == true ]]; then
    cmake --build "$DIR/build"
fi

if [[ "$programmer" == true ]]; then
    openocd  -f interface/cmsis-dap.cfg -c'transport select swd' -c 'adapter speed 5000' -c 'set USE_CORE 0' -f target/rp2040.cfg -c "program ${binary/elf/hex} verify reset" -c "shutdown"
else
    set -x
    if lsusb -d "$vid:$pid" >/dev/null; then
        echo "rebooting to BOOTSEL"
        picotool reboot -u --vid "0x$vid" --pid "0x$pid" -f
        sleep 1
    fi

    if ! lsusb -d "2e8a:0003" >/dev/null; then
        echo "no BOOTSEL dev found, probably not connected I dunno"
        exit 1
    fi
    picotool load -vx "$binary"
fi
