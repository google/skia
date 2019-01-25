#! /bin/sh

# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Run this script after updating ../../DEPS to point at a new revision of ICU.

set -x -e

DATA_TMP="$(mktemp "${TMPDIR:-/tmp}/icu_dat.XXXXXXXXXX";)"
cd "$(dirname "$0")"
python ../../tools/git-sync-deps
python ./build_icu_data_file.py ../externals/icu "$DATA_TMP"
UVERNUM_FILE=../externals/icu/icu4c/source/common/unicode/uvernum.h
UVERNUM_SEDCMD='s/^#define U_ICU_VERSION_MAJOR_NUM \([1-9][0-9]*\)$/\1/p'
SYMBOL="icudt$(sed -n "$UVERNUM_SEDCMD" "$UVERNUM_FILE")_dat"
CHECKSUM="$(python ./hasher.py 'md5' "$DATA_TMP")"

BUCKET='skia-icu-data'
SRC='../externals/icu/icu4c/source'
{
    cat <<- EOM
	# Copyright $(date +%Y) Google LLC.
	# Use of this source code is governed by a BSD-style license that can be
	# found in the LICENSE file.
	icu_dtname = "${SYMBOL}"
	icu_dat_md5 = "${CHECKSUM}"
	icu_dat_bucket = "${BUCKET}"
	_src = "$SRC"
	icu_sources = [
	EOM
    find $SRC/common $SRC/i18n -name '*.cpp' -o -name '*.h' -o -name '*.c' | \
        LANG= sort | \
        sed "s#^${SRC}/\(.*\)\$#  \"\$_src/\1\",#"
    printf ']\n'
} > icu.gni

set +x +e
URL="https://storage.googleapis.com/${BUCKET}/$CHECKSUM"
if [ "$(curl -s -I "$URL" | head -n 1|cut -d' ' -f2)" != 200 ]; then
    echo "TODO: upload the dat file:"
    echo "  gsutil cp '$DATA_TMP' 'gs://${BUCKET}/$CHECKSUM'"
else
    echo "DAT file is already here: $URL"
fi
echo SUCCESS
echo '( git add third_party/icu/icu.gni )'
