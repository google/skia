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
into a static archive (.a [Linux/Mac] or .lib [Windows]).
"""

import argparse
import os
import shutil
import subprocess
import sys

from cmake_utils import (
    add_common_cmake_args, combine_into_library, discover_dependencies,
    get_cmake_os_cpu, get_windows_settings, quote_if_needed, write_depfile,
    get_third_party_locations)


def main():
  parser = argparse.ArgumentParser(description="Build Tint using CMake.")
  add_common_cmake_args(parser)
  args = parser.parse_args()

  target_os, target_cpu = get_cmake_os_cpu(args.target_os, args.target_cpu)

  output_path = args.output_path
  depfile_path = args.depfile_path
  script_dir = os.path.dirname(os.path.realpath(__file__))
  dawn_dir = os.path.join(script_dir, "..", "externals", "dawn")

  # The build can be invoked in parallel for different toolchains, so we use
  # a short, unique build directory.
  build_dir = args.build_dir

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
      f"-DCMAKE_SYSTEM_NAME={target_os}",
      f"-DCMAKE_SYSTEM_PROCESSOR={target_cpu}",
      # This is handled by GN
      "-DDAWN_FETCH_DEPENDENCIES=OFF",
      "-DDAWN_ENABLE_INSTALL=OFF",
      f"-DCMAKE_BUILD_TYPE={args.build_type}",
      # skslc needs WGSL support.
      "-DTINT_BUILD_WGSL_READER=ON",
      "-DTINT_BUILD_WGSL_WRITER=ON",
      "-DTINT_ENABLE_INSTALL=ON",
      # Use Ninja for faster builds.
      "-G",
      "Ninja",
      f"-DCMAKE_MAKE_PROGRAM={ninja_exe}",
      "-DDAWN_ENABLE_D3D11=OFF",
      "-DDAWN_ENABLE_D3D12=OFF",
      "-DDAWN_ENABLE_METAL=OFF",
      "-DDAWN_ENABLE_NULL=OFF",
      "-DDAWN_ENABLE_DESKTOP_GL=OFF",
      "-DDAWN_ENABLE_OPENGLES=OFF",
      "-DDAWN_ENABLE_VULKAN=OFF",
  ]
  configure_cmd += get_third_party_locations()
  if args.enable_rtti:
    configure_cmd.append("-DDAWN_ENABLE_RTTI=ON")

  cxx_flags = args.cxx_flags or []
  ld_flags = args.ld_flags or []

  if target_os == "Windows":
    win_cfgs, win_cxx, win_ld = get_windows_settings(args)
    configure_cmd += win_cfgs
    cxx_flags += win_cxx
    ld_flags += win_ld

  if cxx_flags:
    c_cxx_flags_str = " ".join(cxx_flags)
    configure_cmd.append(f"-DCMAKE_CXX_FLAGS={c_cxx_flags_str}")

  if ld_flags:
    ld_flags_str = " ".join(ld_flags)
    configure_cmd.append(f"-DCMAKE_EXE_LINKER_FLAGS={ld_flags_str}")
    configure_cmd.append(f"-DCMAKE_SHARED_LINKER_FLAGS={ld_flags_str}")
    configure_cmd.append(f"-DCMAKE_MODULE_LINKER_FLAGS={ld_flags_str}")

  # Set PYTHONPATH to include Dawn's third_party directory. This is needed
  # for the generator scripts to find jinja2 and markupsafe as packages.
  third_party_dir = os.path.abspath(os.path.join(dawn_dir, "third_party"))
  env = os.environ.copy()
  env["PYTHONPATH"] = third_party_dir
  # Don't write .pyc files, which can cause race conditions when building
  # tint and dawn in parallel.
  env["PYTHONDONTWRITEBYTECODE"] = "1"

  subprocess.run(configure_cmd, check=True, env=env)

  # These tint targets (and their deps) are what Skia needs to build
  tint_targets = ["tint_api", "tint_lang_wgsl_reader", "tint_lang_wgsl_writer"]

  build_cmd = [ninja_exe, "-C", build_dir, "-dkeepdepfile"] + tint_targets
  subprocess.run(build_cmd, check=True, env=env)

  dependencies, object_files = discover_dependencies(build_dir, tint_targets)
  # Generate the depfile. This lists all source files that the Tint library
  # depends on. This allows GN to know when to re-run this script.
  write_depfile(output_path, depfile_path, dependencies)

  # After building, Tint consists of many small object files. For easier
  # consumption by GN, we combine them into a single archive using ar.
  combine_into_library(args, output_path, build_dir, target_os, object_files)


if __name__ == "__main__":
  main()
