#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Builds Tint using CMake."""

import hashlib
import glob
import os
import re
import shutil
import subprocess
import sys


def discover_dependencies(build_dir, targets):
  """Runs ninja -tinputs recursively to discover all targets, then uses
  ninja -tdeps to get all source files for those targets."""
  # The worklist contains ninja targets we need to find the inputs for.
  worklist = set(targets)
  # We keep track of all ninja targets we've ever seen to avoid cycles and
  # redundant processing.
  seen_targets = set(targets)

  while len(worklist) > 0:
    current_batch = list(worklist)
    worklist.clear()

    cmd = ["ninja", "-C", build_dir, "-tinputs"] + current_batch
    inputs = subprocess.check_output(cmd).decode("utf-8").splitlines()
    # inputs looks like:
    #   /home/user/skia/third_party/externals/dawn/src/tint/utils/text/styled_text_theme.cc
    #   /home/user/skia/third_party/externals/dawn/src/tint/utils/text/unicode.cc
    #   cmake_object_order_depends_target_tint_api_common
    #   cmake_object_order_depends_target_tint_lang_core
    #   src/tint/CMakeFiles/tint_api_common.dir/api/common/vertex_pulling_config.cc.o
    #   src/tint/CMakeFiles/tint_lang_core.dir/lang/core/binary_op.cc.o
    #   src/tint/libtint_api_common.a
    #   src/tint/libtint_lang_core.a
    # Some of these are files (which we can ignore), some are more targets, which we should
    # recursively get targets for to make sure we have the whole dependency graph.
    for line in inputs:
      line = line.strip()
      if line.startswith("/"):
        continue # file, which are always absolute paths, skip it
      if line not in seen_targets:
        worklist.add(line)
        seen_targets.add(line)

  # Now that we have all the targets, get the dependencies for them.
  source_files = set()
  # We pass all seen targets to the deps command. This includes the initial
  # targets and any intermediate targets discovered.
  cmd = ["ninja", "-C", build_dir, "-tdeps"] + list(seen_targets)
  output = subprocess.check_output(cmd).decode("utf-8").splitlines()

  # When a target has deps, which are read from the .d files generated from the "-dkeepdepfile
  # option earlier, the ninja command outputs something like this:
  #   third_party/spirv-tools/source/opt/CMakeFiles/SPIRV-Tools-opt.dir/eliminate_dead_functions_util.cpp.o: #deps 408, deps mtime 1755004530688332629 (VALID)
  #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/eliminate_dead_functions_util.cpp
  #       /usr/include/stdc-predef.h
  #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/eliminate_dead_functions_util.h
  #       /home/user/skia/third_party/externals/dawn/third_party/spirv-tools/src/source/opt/ir_context.h
  #       /usr/include/c++/14/algorithm
  # If there's not a match, it's a simple message like:
  # src/tint/libtint_utils_text_generator.a: deps not found
  # We want to aggregate all the indented files
  for line in output:
    if not line.startswith("  ") or "deps not found" in line:
      continue
    line = line.strip()
    source_files.add(line)

  result = list(source_files)
  result.sort()
  return result


def write_depfile(output_path, depfile_path, dependencies):
  """Generates a .d file that lists all discovered source files as
  dependencies for the given output library."""
  os.makedirs(os.path.dirname(depfile_path), exist_ok=True)
  with open(depfile_path, "w") as f:
    f.write(f"{output_path}:")
    # Make sure to use forward slashes for paths in the depfile, even on
    # Windows, for consistency.
    for dep in dependencies:
      f.write(" \\\n" + dep.replace("\\", "/"))
    f.write("\n")


def copy_if_changed(src, dest):
  """Copies the file from src to dest if dest doesn't exist or if the
  contents of src and dest are different."""
  if os.path.exists(dest):
    src_hash = hashlib.sha256(open(src, "rb").read()).hexdigest()
    dest_hash = hashlib.sha256(open(dest, "rb").read()).hexdigest()
    if src_hash == dest_hash:
      # The files are identical, no need to copy.
      return

  # Either the destination does not exist or it is different.
  shutil.copyfile(src, dest)
  os.chmod(dest, 0o755)


def main():
  if len(sys.argv) != 3:
    print("Usage: build_tint.py <output_path> <depfile_path>", file=sys.stderr)
    sys.exit(1)

  output_path = sys.argv[1]
  depfile_path = sys.argv[2]
  # Assume Dawn is checked out in $SKIA_ROOT/third_party/externals and we
  # are running from out/SomeBuildDirectory
  dawn_dir = os.path.join("..", "..", "third_party", "externals", "dawn")

  # We will build everything in a temporary directory (under out/SomeBuildDirectory).
  # The build can be invoked in parallel for different toolchains, so we use
  # part of the output path to create a unique build directory.
  build_dir = os.path.join("cmake", os.path.dirname(output_path), "dawn")

  # Make sure cmake and ninja are on PATH
  subprocess.run(["cmake", "--version"], check=True)
  subprocess.run(["ninja", "--version"], check=True)

  # Configure the project using CMake. Tint is a part of Dawn.
  # https://github.com/google/dawn/blob/main/docs/quickstart-cmake.md
  configure_cmd = [
      "cmake",
      "-S",
      dawn_dir,
      "-B",
      build_dir,
      # Fetch dependencies using DEPS, which is required for stand-alone builds.
      "-DDAWN_FETCH_DEPENDENCIES=ON",
      "-DDAWN_ENABLE_INSTALL=ON",
      "-DCMAKE_BUILD_TYPE=Release",
      "-DDAWN_USE_X11=OFF",
      # Samples and tests are not needed and may have extra dependencies.
      "-DDAWN_BUILD_SAMPLES=OFF",
      "-DTINT_BUILD_TESTS=OFF",
      # skslc needs WGSL support.
      "-DTINT_BUILD_WGSL_READER=ON",
      "-DTINT_BUILD_WGSL_WRITER=ON",
      "-DTINT_ENABLE_INSTALL=ON",
      # Use Ninja for faster builds.
      "-G",
      "Ninja",
  ]
  subprocess.run(configure_cmd, check=True)

  # These tint targets (and their deps) are what Skia needs to build
  tint_targets = ["tint_api", "tint_lang_wgsl_reader", "tint_lang_wgsl_writer"]

  build_cmd = ["ninja", "-C", build_dir, "-dkeepdepfile"] + tint_targets
  subprocess.run(build_cmd, check=True)

  # Generate the depfile. This lists all source files that the Tint library
  # depends on. This allows GN to know when to re-run this script.
  dependencies = discover_dependencies(build_dir, tint_targets)
  write_depfile(output_path, depfile_path, dependencies)

  # After building, Tint consists of 90+ small static libraries. For easier
  # consumption by GN, we combine them into a single archive using ar.
  tint_libs = glob.glob(os.path.join(build_dir, "src", "tint", "lib*.a"))
  if not tint_libs:
    sys.exit("Error: No static libraries found for Tint.")

  # Tint also depends on Abseil. Combining Tint's deps into the library makes
  # consumption by GN easier.
  absl_libs = [
      os.path.join(build_dir, "third_party", "abseil", "absl", "strings",
                   "libabsl_strings.a")
  ]

  # https://stackoverflow.com/a/23621751
  # We first create a "thin" archive. A thin archive does not contain the
  # object files themselves, but rather references to them.
  #   c: Create the archive.
  #   q: Quickly append files (no checking for replacement).
  #   T: Create a thin archive.
  combined_thin_archive = os.path.join(build_dir, "out", "combined_tint_thin.a")
  os.makedirs(os.path.dirname(combined_thin_archive), exist_ok=True)
  combine_cmd = ["ar", "cqT", combined_thin_archive] + tint_libs + absl_libs
  subprocess.run(combine_cmd, check=True)

  # We'll be relocating the output, so a thin archive doesn't work. To create
  # a regular, "fat" archive, we use an MRI script with 'ar'.
  combined_archive = os.path.join(build_dir, "out", "libtint_combined.a")
  mri_script = f"""
create {combined_archive}
addlib {combined_thin_archive}
save
end
"""
  # The 'ar -M' command reads commands from stdin to modify the archive.
  subprocess.run(["ar", "-M"], input=mri_script.encode("utf-8"), check=True)

  # Copy the final archive to the location expected by GN.
  # We check hashes to avoid unnecessary updates to the "last modified" datum
  # of the file, which can cause GN to unnecessarily rebuild downstream dependencies.
  copy_if_changed(combined_archive, os.path.join(os.getcwd(), output_path))


if __name__ == "__main__":
  main()
