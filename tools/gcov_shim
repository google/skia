#!/bin/bash

# Running gcov with -a (--all-blocks) will hang on some files.  lcov uses -a.
# This shim strips out that flag (a minor feature) so we can run gcov.

CMD="gcov"

while (( "$#" )); do
    if [[ "$1" != "-a" && "$1" != "-all-blocks" && "$1" != "--all-blocks" ]]; then
        CMD="$CMD $1"
    fi
    shift
done

$CMD
