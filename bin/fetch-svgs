#!/bin/sh

# Copies the latest bot-generated SVG set to ./svgs.

set -x
set -e

ROOT_DIR="$(cd $(dirname $0)/..; pwd)"
python3 ${ROOT_DIR}/bin/fetch-sk
case "$(uname -s)" in
  CYGWIN*) SK_EXE="sk.exe";;
  MINGW*)  SK_EXE="sk.exe";;
  *)       SK_EXE="sk";;
esac
${ROOT_DIR}/bin/${SK_EXE} asset download svg $(pwd)/svgs
