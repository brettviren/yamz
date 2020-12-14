#!/bin/bash

what="$1"

tstdir=$(dirname $(realpath $BASH_SOURCE))
topdir=$(dirname $tstdir)

valid () {
    what=$1    
    set -x
    moo -D "${what}.model" -M "$topdir/src" validate \
        --passfail --sequence -S "${what}.valid" \
        -s "$tstdir/test-yamz-re.jsonnet"  \
        "$tstdir/test-yamz-re.jsonnet"
    set +x
}
check () {
    what=$1
    not=FAIL
    if [ "$what" = "fail" ] ; then
        not=PASS
    fi
    res=$(valid $what | grep $not)
    if [ -n "$res" ] ; then
        echo "got $not in $what test"
        exit -1
    fi
}

if [ -n "$what" ] ; then
    valid $what
else
    check pass
    check fail
fi

