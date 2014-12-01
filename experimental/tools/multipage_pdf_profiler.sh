#!/bin/sh

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "usage: $0 SKP_DIRECTORY MULTIPAGE_PDF_PROFILER_EXE" >&2
    exit 1
fi

SKP_DIRECTORY="$1"
MULTIPAGE_PDF_PROFILER_EXE="$2"

printf '"FILE","SKP SIZE (BYTES)","MEMORY USE (MB)"\n' >&2
for skp in "$SKP_DIRECTORY"/*.skp; do
    r=$("$MULTIPAGE_PDF_PROFILER_EXE" 0 "$skp")
    skp_size=$(echo $r | awk '{ print $1 }')
    mem0=$(echo $r | awk '{ print $2 }')
    mem1=$("$MULTIPAGE_PDF_PROFILER_EXE" 1 "$skp" | awk '{ print $2 }')
    printf '"%s",%d,%d\n' $(basename "$skp") $skp_size $(($mem1 - $mem0))
done
