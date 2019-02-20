#! /bin/sh
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

THIS_DIR="$(dirname "$0")"
BLACKLIST="$(printf '%s\|' \
        hb-coretext        \
        hb-directwrite     \
        hb-fallback-shape  \
        hb-ft              \
        hb-glib            \
        hb-gobject         \
        hb-gobject-structs \
        hb-graphite2       \
        hb-ucdn            \
        hb-uniscribe       \
)"
{
    cat <<- EOF
	# Copyright 2019 Google LLC.
	# Use of this source code is governed by a BSD-style license that can be
	# found in the LICENSE file.
	_src = "../externals/harfbuzz/src"
	harfbuzz_sources = [
	EOF
    (
        cd "${THIS_DIR}/../externals/harfbuzz"
        find 'src' -name 'hb*.cc' -o -name 'hb*.hh' -o -name 'hb*.h' | \
            LANG= sort | grep -v "^src/\(${BLACKLIST%\\\|}\)\." | \
            sed 's/^/  "$_/;s/$/",/'
    )
    printf ']\n'
} > "${THIS_DIR}/harfbuzz.gni"
