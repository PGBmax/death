#!/bin/bash

if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo "Usage: $0 <count> [32]"
    exit 1
fi

count="$1"
out_dir="/tmp/test/"
use_32=false

if [ "$#" -eq 2 ]; then
    if [ "$2" = "32" ]; then
        use_32=true
    else
        echo "Second argument must be '32' or omitted"
        exit 1
    fi
fi

if ! [[ "$count" =~ ^[0-9]+$ ]]; then
    echo "Count must be a non-negative integer"
    exit 1
fi

mkdir -p "$out_dir"

cat > /tmp/sample.c <<'EOF'
#include <stdio.h>

int main(void)
{
    printf("Hello World!\n");
    return 0;
}
EOF

for ((i=0; i<count; i++)); do
    output="$out_dir/samples$i"
    if [ "$use_32" = true ]; then
        cc -m32 /tmp/sample.c -o "$output"
    else
        cc /tmp/sample.c -o "$output"
    fi
done

echo "Created $count executables in $out_dir"
