#!/usr/bin/env python
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""
Create bazel_build task_driver. This should rarely need to change, so by putting it into
CIPD, we should reduce build latency by removing the need for the BuildTaskDrivers job.
That job *is* idempotent, so Swarming does eventually deduplicate it sometimes (but not
after a usually unrelated change to something in //bazel), but skipping the step is faster.
"""


import argparse
import os
import shutil
import subprocess

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
SKIA_ROOT_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir, os.pardir, os.pardir))

def create_asset(target_dir):
  """Compiles the task driver using bazel, which is presumed to be on PATH """
  out = subprocess.check_output([
    "bazel", "build", "//infra/bots/task_drivers/bazel_build",
    "--platforms=@io_bazel_rules_go//go/toolchain:linux_amd64",
  ], encoding='utf-8')
  print(out)
  path_to_binary = os.path.join(SKIA_ROOT_DIR, 'bazel-bin', 'infra', 'bots', 'task_drivers',
                                'bazel_build', 'bazel_build_', 'bazel_build')
  shutil.copy(path_to_binary, target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

