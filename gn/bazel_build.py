#!/usr/bin/env python3
#
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This script runs bazel build [target] [additional_arg] [additional_arg]
# and then copies each of the specified outputs from the bazel-bin output
# folder under the current working directory. This allows GN to shell out
# to Bazel to generate or compile files, but still have them show up where
# GN expects (in the output folder).
#
# The first argument passed in to this script is the Bazel target.
# subsequent arguments are treated either as Bazel flags (if they start with
# --) or output files to copy (if they do not).
#
# Output files may contain an equals sign (=), which means the first part is
# the Bazel file to copy and the second part is where to copy it. If omitted,
# the file will go immediately under the GN output directory.

import hashlib
import os
import shutil
import subprocess
import sys

target = sys.argv[1]
outputs = {}
bazel_args = []
for arg in sys.argv[2:]:
    if arg.startswith("--"):
        bazel_args.append(arg)
    else:
        if "=" in arg:
            # "../../bazel-bin/src/ports/ffi.h=src/core/ffi.h" means to
            # copy ../../bazel-bin/src/ports/ffi.h to
            # $GN_OUTPUT/src/core/ffi.h
            bazel_file, output_path = arg.split("=")
            outputs[bazel_file] = output_path
        else:
            # "../../bazel-bin/src/ports/libstuff.a" means to copy
            # ../../bazel-bin/src/ports/libstuff.a to
            # $GN_OUTPUT/libstuff.a
            outputs[arg] = os.path.basename(arg)

print("Invoking bazelisk from ", os.getcwd())
# Forward the remaining args to the bazel invocation
subprocess.run(["bazelisk", "build", target ] + bazel_args, check=True)

for bazel_file, output_path in outputs.items():
    # GN expects files to be created underneath the output directory from which
    # this script is invoked.
    expected_output = os.path.join(os.getcwd(), output_path)
    if not os.path.exists(expected_output):
        shutil.copyfile(bazel_file, expected_output)
        os.chmod(expected_output, 0o755)
    else:
        # GN uses timestamps to determine if it should re-build a file. If the
        # two files match (via hash) we should not re-copy the output, which would
        # re-trigger subsequent rebuilds.
        created_hash = hashlib.sha256(open(bazel_file, 'rb').read()).hexdigest()
        existing_hash = hashlib.sha256(open(expected_output, 'rb').read()).hexdigest()
        if created_hash != existing_hash:
            os.remove(expected_output)
            shutil.copyfile(bazel_file, expected_output)
            os.chmod(expected_output, 0o755)
