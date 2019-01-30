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
CPPFLAGS="$ICUFLAGS" "${ICU_DIR}/source/runConfigureICU" $OS \
    --enable-static --disable-shared --with-data-packaging=archive
make -j

DATA_FILE="$(find data/out -name 'icudt*l.dat' | head -n 1)"

test -f "$DATA_FILE"

FILENAME="$(basename "$DATA_FILE")"

SYMBOL="$(basename "$DATA_FILE" 'l.dat')_dat"

DATA_TMP="$(mktemp)"

mv "$DATA_FILE" "$DATA_TMP"

cd "$SKIA_DIR"/third_party/icu

CHECKSUM="$(python hasher.py md5 "$DATA_TMP")"

rm -rf "$T"

BUCKET='skia-icu-data'

{
    cat <<- EOM
	# Copyright $(date +%Y) Google Inc.
	# Use of this source code is governed by a BSD-style license that can be
	# found in the LICENSE file.
	icu_dtname = "${SYMBOL}"
	icu_dat_md5 = "${CHECKSUM}"
	icu_dat_bucket = "${BUCKET}"
	icu_sources = [
	EOM
    find ../externals/icu/icu4c/source/common \
         ../externals/icu/icu4c/source/i18n   \
         -name '*.cpp' | LANG= sort | sed 's/^/  "/;s/$/",/'
    printf ']\n'
} > icu.gni

URL="https://storage.googleapis.com/${BUCKET}/$CHECKSUM"
if [ "$(curl -s -I "$URL" | head -n 1|cut -d' ' -f2)" != 200 ]; then
    echo TODO: upload the dat file:
    echo gsutil cp "$DATA_TMP" "gs://${BUCKET}/$CHECKSUM"
fi
echo SUCCESS
echo git add 'third_party/icu/icu.gni'
