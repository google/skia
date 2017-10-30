#!/bin/sh
# Copyright 2017 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# If libraries are located in a non-standard location (e.g. ~/local or ~/homebrew),
# amend $CPPFLAGS & $LDFLAGS to point there, then run `make`.
if [ "$1" ]; then
  export CPPFLAGS="${CPPFLAGS} -I \"${1}/include\""
  export LDFLAGS="${LDFLAGS} -L \"${1}/lib\" -Wl,-rpath -Wl,\"${1}/lib\""
fi
shift
make
