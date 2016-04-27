#!/bin/sh

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This tool compares the PDF output of Skia's DM tool of two commits.

CONTROL_COMMIT="$1"
EXPERIMENT_COMMIT="$2"

SOURCE="${3:-gm}" # could be 'skp'

if ! [ "$1" ] || ! [ "$2" ]; then
    echo "usage:" >&2
    echo "  $0 CONTROL_COMMIT EXPERIMENT_COMMIT [SOURCE]" >&2
    exit 1
fi

BAD=''
for CMD in 'python' 'ninja' 'pdfium_test' 'skdiff'; do
    if ! command -v "$CMD" > /dev/null ; then
        echo "could not find $CMD command in PATH." >&2
        BAD=1
    fi
done
if [ "$BAD" ]; then exit 1; fi

cd "$(dirname "$0")/.."
if [ "$(git diff --shortstat)" ]; then
    echo "please stash your changes" >&2
    exit 1
fi

DIR=$(mktemp -d "${TMPDIR:-/tmp}/skpdf.XXXXXXXXXX")
EXP="${DIR}/exp"
CON="${DIR}/con"

set -e

git checkout "$EXPERIMENT_COMMIT"
python bin/sync-and-gyp && ninja -C out/Release dm
out/Release/dm --src "$SOURCE" --config pdf -w "$EXP"

git checkout "$CONTROL_COMMIT"
python bin/sync-and-gyp && ninja -C out/Release dm
out/Release/dm --src "$SOURCE" --config pdf -w "$CON"

set +e

EXP_DIR="${EXP}/pdf/${SOURCE}"
CON_DIR="${CON}/pdf/${SOURCE}"

DIFFS=''
# remove byte-identical PDFs
for con in "$CON_DIR"/*pdf; do
    exp="$EXP_DIR/$(basename "$con")"
    if diff "$con" "$exp" > /dev/null; then
        rm "$con" "$exp" # no difference
    else
        echo "PDF differs: $(basename "$con")"
        DIFFS=1
    fi
done
if [ -z "$DIFFS" ]; then
    echo 'All PDFs are byte-identical!'
    rm -r "$DIR"
    exit 0;
fi

# Portable version of timeout from GNU coreutils.
timeout_py() { python -c "$(cat <<EOF
import sys, subprocess, threading
proc = subprocess.Popen(sys.argv[2:])
timer = threading.Timer(float(sys.argv[1]), proc.terminate)
timer.start()
proc.wait()
timer.cancel()
exit(proc.returncode)
EOF
)" "$@"; }

# rasterize the remaining PDFs
for pdf in "$CON_DIR"/*pdf "$EXP_DIR"/*pdf ; do
    if timeout_py 10 pdfium_test --png "$pdf"; then
        if ! [ -f "$pdf".*.png ] ; then
            echo "Missing pdfium_test output: '$pdf.*.png'" >&2
            exit 1
        fi
        rm "$pdf"
    else
        echo "pdfium_test '$pdf' failed."
    fi
done

DIFFS=''
# remove byte-identical PNGs:
for con in "$CON_DIR"/*.png; do
    exp="$EXP_DIR/$(basename "$con")"
    if diff "$con" "$exp"; then
        rm "$exp" "$con"
    else
        echo "PNG differs: $(basename "$con")"
        DIFFS=1
    fi
done
if [ -z "$DIFFS" ]; then
    echo 'All PNGs are byte-identical!'
    rm -r "$DIR"
    exit 0;
fi

# run remaining PNG files through skdiff:
DIFF_DIR="${DIR}/skdiffout"
skdiff "$CON_DIR" "$EXP_DIR"  "$DIFF_DIR"
echo "'$DIFF_DIR/index.html'"

if [ $(uname) = 'Darwin' ] ; then
    open "$DIFF_DIR/index.html"  # look at diffs
elif [ $(uname) = 'Linux' ] ; then
    xdg-open "$DIFF_DIR/index.html"  # look at diffs
fi
