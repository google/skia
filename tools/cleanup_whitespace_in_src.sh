#! /bin/sh
# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

sed_in_place() {
    # works the same with GNU and BSD sed.
    local expr="$1"
    local file="$2"
    local t="$(mktemp "$(dirname "$file")/tmp.XXXXXXXXXX")"
    sed "$expr" < "$file" > "$t"
    mv "$t" "$file"
}

IGNORE='^third_party/vulkanmemoryallocator/'

c_files_only() { grep '\.\(c\|h\|cpp\|cxx\|cc\)$' | grep -v "$IGNORE"; }

for file in $(git grep -l $'\r' | c_files_only); do
    sed_in_place $'s/\r//' "$file"
done

for file in $(git grep -l $'[ \t]$' | c_files_only); do
    sed_in_place $'s/[ \t][ \t]*$//' "$file"
done

git ls-files | c_files_only | xargs chmod -x

