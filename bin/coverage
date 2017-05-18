#!/bin/sh
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [ -z "$1" ]; then
    cat <<-EOM
	Usage:
	  $0 SKIA_EXECUTABLE [ARGUMENTS_FOR_EXECUTABLE...]

	Run something like this:
	  $0 dm --src tests
	or
	  $0 dm --src gm skp

	EOM
    exit 1
fi

set -x
set -e

cd "$(dirname "$0")/.."

EXECUTABLE="$1"
shift

DIR="$(mktemp -d "${TMPDIR:-/tmp}/skia_coverage_XXXXXXXXXX")"
BUILD=out/coverage

# Build $EXECUTABLE
bin/sync
bin/fetch-gn

#TODO: make this work with Clang.
ARGS='cc="gcc" cxx="g++" extra_cflags=["--coverage"] extra_ldflags=["--coverage"]'
gn gen --args="$ARGS" "$BUILD"

ninja -C "$BUILD" "$EXECUTABLE"

GCOV="$(realpath tools/gcov_shim)"

# Generate a zero-baseline so files not covered by $EXECUTABLE $@ will
# still show up in the report.  This reads the .gcno files that are
# created at compile time.
lcov -q --gcov-tool="$GCOV" -c -b "$BUILD" -d "$BUILD" -o "$DIR"/baseline -i

# Running the binary generates the real coverage information, the .gcda files.
"$BUILD"/"$EXECUTABLE" "$@"

lcov -q --gcov-tool="$GCOV" -c -b "$BUILD" -d "$BUILD" -o "$DIR"/coverage

lcov -q -a "$DIR"/baseline -a "$DIR"/coverage -o "$DIR"/merged

genhtml -q "$DIR"/merged --legend -o "$DIR"/coverage_report --ignore-errors source

xdg-open "$DIR"/coverage_report/index.html
