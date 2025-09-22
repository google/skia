#!/usr/bin/env python3
#
# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Builds Tint using CMake.

This script configures and builds Tint using CMake and Ninja. It then
discovers all object files for the specified libraries and combines them
into static archives (.a files) using `ar`.
"""

import argparse
import os
import shutil
import subprocess
import sys

from cmake_utils import (copy_if_changed, discover_dependencies,
                         quote_if_needed, write_depfile)

def main():
  parser = argparse.ArgumentParser(description="Build Tint using CMake.")
  parser.add_argument("--cc", required=True, help="Path to the C compiler.")
  parser.add_argument("--cxx", required=True, help="Path to the C++ compiler.")
  parser.add_argument(
      "--cxx_flags",
      default=[],
      action="append",
      help="C++ compiler flags. Can be specified multiple times.")
  parser.add_argument(
      "--ld_flags",
      default=[],
      action="append",
      help="Linker flags. Can be specified multiple times.")
  parser.add_argument(
      "--output_path", required=True, help="Path to the output library.")
  parser.add_argument(
      "--depfile_path", required=True, help="Path to the depfile to generate.")
  parser.add_argument(
      "--target_os", default="", help="Target OS for host compilation.")
  parser.add_argument(
      "--target_cpu", default="", help="Target CPU for host compilation.")
  args = parser.parse_args()

  output_path = args.output_path
  depfile_path = args.depfile_path
  script_dir = os.path.dirname(os.path.realpath(__file__))
  dawn_dir = os.path.join(script_dir, "..", "externals", "dawn")

  # We will build everything in a temporary directory (under out/SomeBuildDirectory).
  # The build can be invoked in parallel for different toolchains, so we use
  # part of the output path to create a unique build directory.
  build_dir = os.path.join("cmake", os.path.dirname(output_path), "dawn_for_tint")

  cmake_exe = shutil.which("cmake")
  if not cmake_exe:
    print("Error: cmake not found in PATH.")
    sys.exit(1)

  ninja_exe = shutil.which("ninja")
  if not ninja_exe:
    print("Error: ninja not found in PATH.")
    sys.exit(1)

  # Configure the project using CMake. Tint is a part of Dawn.
  # https://github.com/google/dawn/blob/main/docs/quickstart-cmake.md
  configure_cmd = [
      cmake_exe,
      "-S",
      dawn_dir,
      "-B",
      build_dir,
      f"-DCMAKE_C_COMPILER={quote_if_needed(args.cc)}",
      f"-DCMAKE_CXX_COMPILER={quote_if_needed(args.cxx)}",
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
      f"-DCMAKE_MAKE_PROGRAM={ninja_exe}",
  ]
  cxx_flags_to_add = []
  if args.cxx_flags:
    cxx_flags_to_add.extend(args.cxx_flags)

  if cxx_flags_to_add:
    cxx_flags = " ".join(cxx_flags_to_add)
    configure_cmd.append(f"-DCMAKE_CXX_FLAGS={cxx_flags}")

  ld_flags = args.ld_flags

  if ld_flags:
    ld_flags_str = " ".join(ld_flags)
    configure_cmd.append(f"-DCMAKE_EXE_LINKER_FLAGS={ld_flags_str}")
    configure_cmd.append(f"-DCMAKE_SHARED_LINKER_FLAGS={ld_flags_str}")
    configure_cmd.append(f"-DCMAKE_MODULE_LINKER_FLAGS={ld_flags_str}")
  subprocess.run(configure_cmd, check=True)

  # These tint targets (and their deps) are what Skia needs to build
  tint_targets = ["tint_api", "tint_lang_wgsl_reader", "tint_lang_wgsl_writer"]

  build_cmd = [ninja_exe, "-C", build_dir, "-dkeepdepfile"] + tint_targets
  subprocess.run(build_cmd, check=True)

  dependencies, object_files = discover_dependencies(build_dir, tint_targets)
  # Generate the depfile. This lists all source files that the Tint library
  # depends on. This allows GN to know when to re-run this script.
  write_depfile(output_path, depfile_path, dependencies)

  # After building, Tint consists of many small object files. For easier
  # consumption by GN, we combine them into a single archive using ar.
  lib_name = os.path.basename(output_path)
  gen_library_path = os.path.join(build_dir, lib_name)
  # ar can fail if the archive already exists, e.g. "archive is malformed"
  if os.path.exists(gen_library_path):
    os.remove(gen_library_path)

  assert len(object_files) > 0
  combine_obj_cmd = ["ar", "rcs", lib_name] + object_files
  subprocess.run(combine_obj_cmd, cwd=build_dir, check=True)

  # Copy the final archive to the location expected by GN.
  # We check hashes to avoid unnecessary updates to the "last modified" datum
  # of the file, which can cause GN to unnecessarily rebuild downstream dependencies.
  copy_if_changed(gen_library_path, os.path.join(os.getcwd(), output_path))


if __name__ == "__main__":
  main()
