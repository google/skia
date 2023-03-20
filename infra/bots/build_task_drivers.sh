#!/bin/bash
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Takes two arguments.
#   First argument is the output directory where executables are to be placed.
#   Second (optional) argument is the target platform. These are formatted as os_arch
#   https://github.com/bazelbuild/rules_go/blob/e9a7054ff11a520e3b8aceb76a3ba44bb8da4c94/go/toolchain/toolchains.bzl#L22

set -x -e

# Navigate to the root of the infra checkout.
cd $(dirname ${BASH_SOURCE[0]})
cd ../..

PLATFORM=${2:-linux_amd64} # use linux_amd64 if not specified

# Build the executables and extract them to the folder in the first argument.
# We specify the cache directory to be somewhere other than the default (home directory)
# Because the home directory is mounted on / which typically does not have a lot of disk space.
# /mnt/pd0 is the bigger disk mounted to the GCE VM.
# https://bazel.build/docs/output_directories#layout
bazelisk --output_user_root=/mnt/pd0/bazel_cache \
    build //infra/bots:all_task_drivers --platforms=@io_bazel_rules_go//go/toolchain:${PLATFORM} \
    --config=linux_rbe --jobs=100

tar -xf bazel-bin/infra/bots/built_task_drivers.tar -C ${1}
# Bazel outputs are write-protected, so we make sure everybody can write them. This way there
# are no expected errors in deleting them later.
chmod 0777 ${1}/*
