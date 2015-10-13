#!/bin/sh

# This is a replacement for pkg-config that compares the string passed
# to the --exists argument with the PKG_CONFIG_PATH environment variable
# and returns 1 if they are different.

case $1 in
  --version)
    echo "0.0-cmake-dummy"
    ;;
  --exists)
    shift
    echo "Expected: $@"
    echo "Found:    ${PKG_CONFIG_PATH}"
    [ "$@" = "${PKG_CONFIG_PATH}" ] || exit 1
    ;;
  *)
    exit 255
    ;;
esac
