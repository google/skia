#! /bin/sh

# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -x -e

SKIA_DIR="$(cd "$(dirname "$0")/../.."; pwd)"
ICU_DIR="$SKIA_DIR"/third_party/externals/icu/icu4c

#python "$SKIA_DIR"/tools/git-sync-deps
#
#cd "$(mktemp -d)"
#
#ICUFLAGS="-DU_CHARSET_IS_UTF8=1 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1 -DU_HIDE_OBSOLETE_UTF_OLD_H=1"
#CPPFLAGS="$ICUFLAGS" "${ICU_DIR}/source/runConfigureICU" Linux --enable-static --disable-shared
#make -j
#
#ASSEMBLY="$(find data/out/tmp -name '*_dat.S' | head -1)"
#
#test -f "$ASSEMBLY"
#
#python "$SKIA_DIR"/third_party/icu/s_to_dat.py "$ASSEMBLY" "$SKIA_DIR"/third_party/icu/icu.dat

cd "$SKIA_DIR"/third_party/icu

{
cat << EOM
# Copyright $(date +%Y) Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
icu_dtname = "icudt63_dat"
icu_sources = [
EOM
find ../externals/icu/icu4c/source/common \
     ../externals/icu/icu4c/source/i18n \
     -name '*.cpp' -printf '  "%p",\n' | sort
printf ']\n'
} > icu.gni


