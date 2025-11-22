#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

import argparse
import os
import subprocess
import sys

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("--stamp_path", required=True, help="Path to the output stamp file.")
  parser.add_argument("--depfile_path", required=True, help="Path to the output depfile.")
  parser.add_argument("--dawn_dir", required=True, help="Path to the Dawn checkout.")
  args = parser.parse_args()

  fetch_script = os.path.join(args.dawn_dir, "tools", "fetch_dawn_dependencies.py")
  subprocess.check_call([sys.executable, fetch_script, "--directory=" + args.dawn_dir])

  # Create the stamp file.
  with open(args.stamp_path, "w") as f:
    f.write("done")

  # If the DEPS file changes, GN will know to re-run this script and update the dependencies.
  deps_file = os.path.join(args.dawn_dir, "DEPS")
  with open(args.depfile_path, "w") as f:
    f.write(f"{args.stamp_path}: {deps_file}\n")

if __name__ == "__main__":
  main()
