#!/bin/bash

# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Helper script to serve Bazel undeclared test outputs over HTTP.

if [ $# -ne 1 ]
then
  echo "Usage: $0 <path to outputs.zip>"
  echo
  echo "This is a helper script to serve Bazel undeclared test outputs over HTTP. See the"
  echo "TEST_UNDECLARED_OUTPUTS_DIR environment variable as described in"
  echo "https://bazel.build/reference/test-encyclopedia#initial-conditions."
  echo
  echo "A typical use case is to view the PNG files produced by a GM executed with \"bazel test\"."
  echo "However, this script works with any Bazel target that produces undeclared outputs."
  echo
  echo "Suppose //path/to:some_test is a Bazel target that produces undeclared test outputs. Its"
  echo "undeclared test outputs are typically found inside a ZIP file named"
  echo "bazel-testlogs/path/to/some_test/test.outputs/outputs.zip (relative to the repository's"
  echo "root directory)."
  echo
  echo "Example session:"
  echo
  echo "    $ bazel test //path/to:some_test"
  echo "    $ $0 bazel-testlogs/path/to/some_test/test.outputs/outputs.zip"
  echo "    Serving HTTP on 0.0.0.0 port 8000 (http://0.0.0.0:8000/) ..."
  echo
  exit 1
fi

# Create a temporary directory where we will extract the ZIP file, and delete it on exit.
TMP_DIR="$(mktemp -d)"
trap "rm -rf $TMP_DIR" EXIT

set -x -e

unzip -d $TMP_DIR $1
cd $TMP_DIR && python3 -m http.server
