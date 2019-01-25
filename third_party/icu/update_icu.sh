#! /bin/sh

# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Run this script after updating ../../DEPS to point at a new revision of ICU.

set -x -e

SKIA_DIR="$(cd "$(dirname "$0")/../.."; pwd)"
ICU_DIR="$SKIA_DIR"/third_party/externals/icu/icu4c

python "$SKIA_DIR"/tools/git-sync-deps

T="$(mktemp -d)"
cd "$T"

case "$(uname)" in
    Linux) OS=Linux;;
    Darwin) OS=MacOSX;;
    *) exit 1;;
esac

ICUFLAGS="-DU_CHARSET_IS_UTF8=1 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1"
CPPFLAGS="$ICUFLAGS" "${ICU_DIR}/source/runConfigureICU" $OS --enable-static --disable-shared
make -j

ASSEMBLY="$(find data/out/tmp -name '*_dat.S' | head -n 1)"

test -f "$ASSEMBLY"

NAME="$(grep '^[A-Za-z0-9_]*:$' < "$ASSEMBLY" | sed 's/://' | head -n 1)"

test "$NAME"

if [ $OS = MacOSX ]; then
    NAME="$(echo "$NAME" | sed 's/^_//')"
fi

python "$SKIA_DIR"/third_party/icu/s_to_dat.py "$ASSEMBLY" "$SKIA_DIR"/third_party/icu/icu.dat

cd "$SKIA_DIR"/third_party/icu

rm -rf "$T"

{
    cat <<- EOM
	# Copyright $(date +%Y) Google Inc.
	# Use of this source code is governed by a BSD-style license that can be
	# found in the LICENSE file.
	icu_dtname = "${NAME}"
	icu_sources = [
	EOM
    find ../externals/icu/icu4c/source/common \
         ../externals/icu/icu4c/source/i18n   \
         -name '*.cpp' | LANG= sort | sed 's/^/  "/;s/$/",/'
    printf ']\n'
} > icu.gni


