#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <count>"
    exit 1
fi

count="$1"
out_dir="/tmp/test/"

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
    cc /tmp/sample.c -o "$output"
done

echo "Created $count executables in $out_dir"
