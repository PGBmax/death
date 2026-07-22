#!/bin/bash

# Compares machine code of 2 binaries

usage () {
    echo "cmp_bin <bin1> <bin2>"
}

if [[ $# != 2 ]]
then
    usage
    exit 1
fi

BIN1="${1}"
BIN2="${2}"

RESULT=`xxd $BIN1 > samp1 ; xxd $BIN2 > samp2 ; diff -y --suppress-common-lines samp1 samp2 | grep '^' | wc -l`

diff --suppress-common-lines samp1 samp2

echo Diff: $RESULT

rm -rf samp1
rm -rf samp2

